#pragma once

#include <d3d9.h>

struct ScopedStateBlock {
    IDirect3DDevice9* dev = nullptr;
    IDirect3DStateBlock9* sb = nullptr;
    explicit ScopedStateBlock(IDirect3DDevice9* d) : dev(d) {
        if (dev) {
            dev->CreateStateBlock(D3DSBT_ALL, &sb);
            if (sb) sb->Capture();
        }
    }
    ~ScopedStateBlock() {
        if (sb) { sb->Apply(); sb->Release(); }
    }
};
