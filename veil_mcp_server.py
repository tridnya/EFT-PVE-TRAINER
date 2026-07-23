#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import struct
import sys
import time
from typing import Any


protocol_magic = 0x504D4C56
protocol_version = 1
default_pipe_name = r"\\.\pipe\veil_mcp"
server_name = "veil-mcp"
server_version = "0.1.0"
frame_struct = struct.Struct("<IHHII")

command_status = 1
command_list_assemblies = 2
command_find_classes = 3
command_inspect_class = 4
command_find_objects = 5
command_inspect_object = 6
command_read_field = 7
command_read_collection = 8
command_get_components = 9
command_get_hierarchy = 10
command_write_field = 11
command_read_memory = 12

value_kind_boolean = 1
value_kind_signed_integer = 2
value_kind_unsigned_integer = 3
value_kind_floating_point = 4

JsonObject = dict[str, Any]
request_sequence = 0


class McpError(Exception):
    def __init__(self, message: str, code: int = -32000) -> None:
        super().__init__(message)
        self.code = code


def read_exact(file_handle: Any, size: int) -> bytes:
    chunks: list[bytes] = []
    remaining = size
    while remaining > 0:
        chunk = file_handle.read(remaining)
        if not chunk:
            raise McpError("pipe closed before the response was complete")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def read_content_length(header: bytes) -> int:
    for line in header.decode("ascii", errors="replace").splitlines():
        name, _, value = line.partition(":")
        if name.lower() == "content-length":
            return int(value.strip())
    raise McpError("missing Content-Length header", -32600)


def read_mcp_message() -> JsonObject | None:
    header = bytearray()
    input_stream = sys.stdin.buffer
    while True:
        line = input_stream.readline()
        if line == b"":
            return None
        if line in (b"\r\n", b"\n") and not header:
            continue
        if line in (b"\r\n", b"\n"):
            break
        header.extend(line)

    content_length = read_content_length(bytes(header))
    payload = read_exact(input_stream, content_length)
    message = json.loads(payload.decode("utf-8"))
    if not isinstance(message, dict):
        raise McpError("invalid JSON-RPC message", -32600)
    return message


def write_mcp_message(message: JsonObject) -> None:
    payload = json.dumps(
        message, separators=(",", ":"), ensure_ascii=False
    ).encode("utf-8")
    header = f"Content-Length: {len(payload)}\r\n\r\n".encode("ascii")
    output_stream = sys.stdout.buffer
    output_stream.write(header)
    output_stream.write(payload)
    output_stream.flush()


def make_jsonrpc_result(request_id: Any, result: JsonObject) -> JsonObject:
    return {"jsonrpc": "2.0", "id": request_id, "result": result}


def make_jsonrpc_error(
    request_id: Any, code: int, message: str
) -> JsonObject:
    return {
        "jsonrpc": "2.0",
        "id": request_id,
        "error": {"code": code, "message": message},
    }


def make_text_result(value: JsonObject, is_error: bool = False) -> JsonObject:
    return {
        "content": [
            {
                "type": "text",
                "text": json.dumps(value, indent=2, ensure_ascii=False),
            }
        ],
        "isError": is_error,
    }


def make_schema(
    properties: JsonObject, required: list[str] | None = None
) -> JsonObject:
    return {
        "type": "object",
        "properties": properties,
        "required": required or [],
        "additionalProperties": False,
    }


def address_schema(description: str) -> JsonObject:
    return {
        "description": description,
        "anyOf": [{"type": "integer"}, {"type": "string"}],
    }


def bounded_integer_schema(
    description: str, default: int, minimum: int, maximum: int
) -> JsonObject:
    return {
        "type": "integer",
        "description": description,
        "default": default,
        "minimum": minimum,
        "maximum": maximum,
    }


def get_tool_definitions() -> list[JsonObject]:
    object_address = address_schema(
        "Live IL2CPP object address returned by another Veil MCP tool."
    )
    class_address = address_schema(
        "IL2CPP class metadata address returned by veil_find_classes."
    )
    return [
        {
            "name": "veil_status",
            "description": (
                "Inspect the live Veil bridge, process, Unity thread, raid state, "
                "module bases, and current camera."
            ),
            "inputSchema": make_schema({}),
        },
        {
            "name": "veil_list_assemblies",
            "description": "List loaded IL2CPP assembly images in the live process.",
            "inputSchema": make_schema({}),
        },
        {
            "name": "veil_find_classes",
            "description": (
                "Search classes in one loaded IL2CPP assembly image by namespace "
                "or class-name substring."
            ),
            "inputSchema": make_schema(
                {
                    "assembly": {
                        "type": "string",
                        "description": "Image name, for example Assembly-CSharp.dll.",
                    },
                    "query": {
                        "type": "string",
                        "description": "Case-insensitive full-name substring.",
                        "default": "",
                    },
                    "limit": bounded_integer_schema(
                        "Maximum returned classes.", 50, 1, 500
                    ),
                },
                ["assembly"],
            ),
        },
        {
            "name": "veil_inspect_class",
            "description": (
                "Inspect bounded fields and methods for IL2CPP class metadata."
            ),
            "inputSchema": make_schema(
                {
                    "class_address": class_address,
                    "max_fields": bounded_integer_schema(
                        "Maximum inherited fields.", 128, 1, 500
                    ),
                    "max_methods": bounded_integer_schema(
                        "Maximum inherited methods.", 128, 1, 500
                    ),
                },
                ["class_address"],
            ),
        },
        {
            "name": "veil_find_objects",
            "description": (
                "Find live UnityEngine.Object instances for an exact IL2CPP class, "
                "optionally filtered by Unity object name."
            ),
            "inputSchema": make_schema(
                {
                    "assembly": {
                        "type": "string",
                        "description": "Image name containing the class.",
                    },
                    "namespace": {
                        "type": "string",
                        "description": "Exact IL2CPP namespace; use an empty string when absent.",
                        "default": "",
                    },
                    "class_name": {
                        "type": "string",
                        "description": "Exact IL2CPP class name.",
                    },
                    "name_filter": {
                        "type": "string",
                        "description": "Optional case-insensitive Unity object-name substring.",
                        "default": "",
                    },
                    "limit": bounded_integer_schema(
                        "Maximum returned live objects.", 100, 1, 500
                    ),
                },
                ["assembly", "class_name"],
            ),
        },
        {
            "name": "veil_inspect_object",
            "description": (
                "Inspect one live IL2CPP object, including bounded field values, "
                "methods, Unity name, GameObject, Transform, and position."
            ),
            "inputSchema": make_schema(
                {
                    "object_address": object_address,
                    "max_fields": bounded_integer_schema(
                        "Maximum inherited fields.", 128, 1, 500
                    ),
                    "max_methods": bounded_integer_schema(
                        "Maximum inherited methods.", 64, 1, 500
                    ),
                },
                ["object_address"],
            ),
        },
        {
            "name": "veil_read_field",
            "description": "Read one named field from a live IL2CPP object.",
            "inputSchema": make_schema(
                {
                    "object_address": object_address,
                    "field_name": {
                        "type": "string",
                        "description": "Exact managed field name.",
                    },
                },
                ["object_address", "field_name"],
            ),
        },
        {
            "name": "veil_read_collection",
            "description": (
                "Read bounded elements from a live managed array or List<T>. "
                "Unsupported collection layouts return their count and can be "
                "continued through field inspection."
            ),
            "inputSchema": make_schema(
                {
                    "collection_address": address_schema(
                        "Managed array or collection object address."
                    ),
                    "limit": bounded_integer_schema(
                        "Maximum returned elements.", 100, 1, 500
                    ),
                },
                ["collection_address"],
            ),
        },
        {
            "name": "veil_get_components",
            "description": (
                "Enumerate bounded Unity Components on a live GameObject or Component."
            ),
            "inputSchema": make_schema(
                {
                    "object_address": object_address,
                    "limit": bounded_integer_schema(
                        "Maximum returned components.", 100, 1, 500
                    ),
                },
                ["object_address"],
            ),
        },
        {
            "name": "veil_get_hierarchy",
            "description": (
                "Inspect a bounded child Transform hierarchy from a live GameObject "
                "or Component."
            ),
            "inputSchema": make_schema(
                {
                    "object_address": object_address,
                    "depth": bounded_integer_schema(
                        "Maximum child depth.", 3, 1, 12
                    ),
                    "max_nodes": bounded_integer_schema(
                        "Maximum returned hierarchy nodes.", 200, 1, 500
                    ),
                },
                ["object_address"],
            ),
        },
        {
            "name": "veil_write_field",
            "description": (
                "Explicitly write one primitive instance field on the Unity thread. "
                "Reference, string, static, and object-graph writes are rejected."
            ),
            "inputSchema": make_schema(
                {
                    "object_address": object_address,
                    "field_name": {
                        "type": "string",
                        "description": "Exact managed field name.",
                    },
                    "value_type": {
                        "type": "string",
                        "enum": ["bool", "int", "uint", "float"],
                        "description": "Wire value category; it must match the field type.",
                    },
                    "value": {
                        "description": "Primitive value to write.",
                        "anyOf": [
                            {"type": "boolean"},
                            {"type": "integer"},
                            {"type": "number"},
                        ],
                    },
                },
                ["object_address", "field_name", "value_type", "value"],
            ),
        },
        {
            "name": "veil_read_memory",
            "description": (
                "Read up to 4096 bytes from a committed readable range in the Veil "
                "process. Prefer semantic object tools when metadata is available."
            ),
            "inputSchema": make_schema(
                {
                    "address": address_schema("Process virtual address."),
                    "size": bounded_integer_schema(
                        "Number of bytes to read.", 64, 1, 4096
                    ),
                },
                ["address", "size"],
            ),
        },
    ]


def parse_integer(value: Any, name: str) -> int:
    if isinstance(value, bool):
        raise McpError(f"{name} must be an integer or numeric string")
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        try:
            return int(value, 0)
        except ValueError as error:
            raise McpError(f"{name} is not a valid integer: {value}") from error
    raise McpError(f"{name} must be an integer or numeric string")


def pack_string(value: str) -> bytes:
    encoded = value.encode("utf-8")
    return struct.pack("<I", len(encoded)) + encoded


def get_pipe_name() -> str:
    return os.environ.get("VEIL_MCP_PIPE_NAME", default_pipe_name)


def get_pipe_timeout_seconds() -> float:
    value = os.environ.get("VEIL_MCP_PIPE_TIMEOUT_SECONDS", "20")
    try:
        return max(float(value), 0.1)
    except ValueError:
        return 20.0


def connect_pipe() -> Any:
    pipe_name = get_pipe_name()
    deadline = time.monotonic() + get_pipe_timeout_seconds()
    last_error: OSError | None = None
    while time.monotonic() < deadline:
        try:
            return open(pipe_name, "r+b", buffering=0)
        except OSError as error:
            last_error = error
            time.sleep(0.05)
    if last_error is not None:
        raise McpError(f"failed to connect to {pipe_name}: {last_error}")
    raise McpError(f"failed to connect to {pipe_name}")


def send_bridge_request(command: int, payload: bytes = b"") -> JsonObject:
    global request_sequence
    request_sequence = (request_sequence + 1) & 0xFFFFFFFF
    header = frame_struct.pack(
        protocol_magic,
        protocol_version,
        command,
        request_sequence,
        len(payload),
    )

    with connect_pipe() as pipe_handle:
        pipe_handle.write(header)
        if payload:
            pipe_handle.write(payload)
        pipe_handle.flush()

        response_header = read_exact(pipe_handle, frame_struct.size)
        magic, version, response_command, request_id, length = (
            frame_struct.unpack(response_header)
        )
        if magic != protocol_magic:
            raise McpError(f"invalid bridge response magic: 0x{magic:08x}")
        if version != protocol_version:
            raise McpError(f"unsupported bridge protocol version: {version}")
        if response_command != command or request_id != request_sequence:
            raise McpError("bridge response does not match the request")
        if length > 1024 * 1024:
            raise McpError("bridge response exceeds the size limit")
        response_payload = read_exact(pipe_handle, length)

    try:
        response = json.loads(response_payload.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise McpError(f"bridge returned invalid JSON: {error}") from error
    if not isinstance(response, dict):
        raise McpError("bridge response must be a JSON object")
    return response


def call_bridge_tool(name: str, arguments: JsonObject) -> JsonObject:
    if name == "veil_status":
        response = send_bridge_request(command_status)
    elif name == "veil_list_assemblies":
        response = send_bridge_request(command_list_assemblies)
    elif name == "veil_find_classes":
        payload = (
            pack_string(str(arguments["assembly"]))
            + pack_string(str(arguments.get("query", "")))
            + struct.pack("<I", int(arguments.get("limit", 50)))
        )
        response = send_bridge_request(command_find_classes, payload)
    elif name == "veil_inspect_class":
        payload = struct.pack(
            "<QII",
            parse_integer(arguments["class_address"], "class_address"),
            int(arguments.get("max_fields", 128)),
            int(arguments.get("max_methods", 128)),
        )
        response = send_bridge_request(command_inspect_class, payload)
    elif name == "veil_find_objects":
        payload = (
            pack_string(str(arguments["assembly"]))
            + pack_string(str(arguments.get("namespace", "")))
            + pack_string(str(arguments["class_name"]))
            + pack_string(str(arguments.get("name_filter", "")))
            + struct.pack("<I", int(arguments.get("limit", 100)))
        )
        response = send_bridge_request(command_find_objects, payload)
    elif name == "veil_inspect_object":
        payload = struct.pack(
            "<QII",
            parse_integer(arguments["object_address"], "object_address"),
            int(arguments.get("max_fields", 128)),
            int(arguments.get("max_methods", 64)),
        )
        response = send_bridge_request(command_inspect_object, payload)
    elif name == "veil_read_field":
        payload = struct.pack(
            "<Q",
            parse_integer(arguments["object_address"], "object_address"),
        ) + pack_string(str(arguments["field_name"]))
        response = send_bridge_request(command_read_field, payload)
    elif name == "veil_read_collection":
        payload = struct.pack(
            "<QI",
            parse_integer(arguments["collection_address"], "collection_address"),
            int(arguments.get("limit", 100)),
        )
        response = send_bridge_request(command_read_collection, payload)
    elif name == "veil_get_components":
        payload = struct.pack(
            "<QI",
            parse_integer(arguments["object_address"], "object_address"),
            int(arguments.get("limit", 100)),
        )
        response = send_bridge_request(command_get_components, payload)
    elif name == "veil_get_hierarchy":
        payload = struct.pack(
            "<QII",
            parse_integer(arguments["object_address"], "object_address"),
            int(arguments.get("depth", 3)),
            int(arguments.get("max_nodes", 200)),
        )
        response = send_bridge_request(command_get_hierarchy, payload)
    elif name == "veil_write_field":
        value_type = str(arguments["value_type"])
        value = arguments["value"]
        if value_type == "bool":
            encoded_value = struct.pack(
                "<BB", value_kind_boolean, 1 if bool(value) else 0
            )
        elif value_type == "int":
            encoded_value = struct.pack(
                "<Bq", value_kind_signed_integer, int(value)
            )
        elif value_type == "uint":
            encoded_value = struct.pack(
                "<BQ", value_kind_unsigned_integer, int(value)
            )
        elif value_type == "float":
            encoded_value = struct.pack(
                "<Bd", value_kind_floating_point, float(value)
            )
        else:
            raise McpError(f"unsupported value_type: {value_type}")
        payload = (
            struct.pack(
                "<Q",
                parse_integer(arguments["object_address"], "object_address"),
            )
            + pack_string(str(arguments["field_name"]))
            + encoded_value
        )
        response = send_bridge_request(command_write_field, payload)
    elif name == "veil_read_memory":
        payload = struct.pack(
            "<QI",
            parse_integer(arguments["address"], "address"),
            int(arguments["size"]),
        )
        response = send_bridge_request(command_read_memory, payload)
    else:
        raise McpError(f"unknown tool: {name}")

    return make_text_result(response, not bool(response.get("ok", False)))


def handle_initialize(message: JsonObject) -> JsonObject:
    request_id = message.get("id")
    params = message.get("params", {})
    protocol = "2024-11-05"
    if isinstance(params, dict) and isinstance(
        params.get("protocolVersion"), str
    ):
        protocol = params["protocolVersion"]
    return make_jsonrpc_result(
        request_id,
        {
            "protocolVersion": protocol,
            "capabilities": {"tools": {"listChanged": False}},
            "serverInfo": {"name": server_name, "version": server_version},
        },
    )


def handle_tools_list(message: JsonObject) -> JsonObject:
    return make_jsonrpc_result(
        message.get("id"), {"tools": get_tool_definitions()}
    )


def handle_tools_call(message: JsonObject) -> JsonObject:
    request_id = message.get("id")
    params = message.get("params", {})
    if not isinstance(params, dict):
        return make_jsonrpc_error(request_id, -32602, "params must be an object")

    name = params.get("name")
    arguments = params.get("arguments", {})
    if not isinstance(name, str):
        return make_jsonrpc_error(
            request_id, -32602, "tool name must be a string"
        )
    if arguments is None:
        arguments = {}
    if not isinstance(arguments, dict):
        return make_jsonrpc_error(
            request_id, -32602, "tool arguments must be an object"
        )

    try:
        result = call_bridge_tool(name, arguments)
        return make_jsonrpc_result(request_id, result)
    except McpError as error:
        return make_jsonrpc_result(
            request_id,
            make_text_result({"ok": False, "error": str(error)}, True),
        )
    except Exception as error:
        return make_jsonrpc_result(
            request_id,
            make_text_result(
                {"ok": False, "error": f"unexpected error: {error}"}, True
            ),
        )


def handle_request(message: JsonObject) -> JsonObject | None:
    if "id" not in message:
        return None
    method = message.get("method")
    if method == "initialize":
        return handle_initialize(message)
    if method == "tools/list":
        return handle_tools_list(message)
    if method == "tools/call":
        return handle_tools_call(message)
    if method == "ping":
        return make_jsonrpc_result(message.get("id"), {})
    return make_jsonrpc_error(
        message.get("id"), -32601, f"method not found: {method}"
    )


def main() -> int:
    while True:
        try:
            message = read_mcp_message()
            if message is None:
                return 0
            response = handle_request(message)
            if response is not None:
                write_mcp_message(response)
        except Exception as error:
            print(f"[veil-mcp] {error}", file=sys.stderr, flush=True)
            return 1


if __name__ == "__main__":
    raise SystemExit(main())
