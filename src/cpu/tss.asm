; SPDX-License-Identifier: GPL-3.0-or-later
; Function for loading the TSS.
; Copyright (C) 2026 lilaf

bits 64

global load_tss
load_tss:
        mov ax, 0x30
        ltr ax
        ret
