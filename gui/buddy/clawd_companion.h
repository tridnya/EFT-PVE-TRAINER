#pragma once

struct ID3D11Device;

namespace clawd_companion
{
    void render(ID3D11Device* device);
    void release() noexcept;
}
