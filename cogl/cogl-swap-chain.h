/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2011 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if !defined(__COGL_H_INSIDE__) && !defined(COGL_COMPILATION)
#error "Only <cogl/cogl.h> can be included directly."
#endif

#ifndef __COGL_SWAP_CHAIN_H__
#define __COGL_SWAP_CHAIN_H__

#include <cogl/cogl-types.h>

#ifdef COGL_HAS_GTYPE_SUPPORT
#include <glib-object.h>
#endif

COGL_BEGIN_DECLS

typedef struct _CoglSwapChain CoglSwapChain;

#ifdef COGL_HAS_GTYPE_SUPPORT
/**
 * cogl_swap_chain_get_gtype:
 *
 * Returns: a #GType that can be used with the GLib type system.
 */
GType cogl_swap_chain_get_gtype (void);
#endif

CoglSwapChain *
cogl_swap_chain_new (void);

void
cogl_swap_chain_set_has_alpha (CoglSwapChain *swap_chain,
                               CoglBool has_alpha);

void
cogl_swap_chain_set_length (CoglSwapChain *swap_chain,
                            int length);

CoglBool
cogl_is_swap_chain (void *object);

COGL_END_DECLS

#endif /* __COGL_SWAP_CHAIN_H__ */
