/*
 * Copyright 2008 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
 * Copyright 2009 Jerome Glisse.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie
 *          Alex Deucher
 *          Jerome Glisse
 */
#include "drmP.h"
#include "radeon_reg.h"
#include "radeon.h"

/* rs780  depends on : */
void rs600_mc_disable_clients(struct radeon_device *rdev);

/* This files gather functions specifics to:
 * rs780
 *
 * Some of these functions might be used by newer ASICs.
 */
int rs780_mc_wait_for_idle(struct radeon_device *rdev);
void rs780_gpu_init(struct radeon_device *rdev);


/*
 * MC
 */
int rs780_mc_init(struct radeon_device *rdev)
{
	rs780_gpu_init(rdev);
	/* FIXME: implement */

	rs600_mc_disable_clients(rdev);
	if (rs780_mc_wait_for_idle(rdev)) {
		printk(KERN_WARNING "Failed to wait MC idle while "
		       "programming pipes. Bad things might happen.\n");
	}
	return 0;
}

void rs780_mc_fini(struct radeon_device *rdev)
{
	/* FIXME: implement */
}


/*
 * Global GPU functions
 */
void rs780_errata(struct radeon_device *rdev)
{
	rdev->pll_errata = 0;
}

int rs780_mc_wait_for_idle(struct radeon_device *rdev)
{
	/* FIXME: implement */
	return 0;
}

void rs780_gpu_init(struct radeon_device *rdev)
{
	/* FIXME: implement */
}


/*
 * VRAM info
 */
void rs780_vram_get_type(struct radeon_device *rdev)
{
	/* FIXME: implement */
}

void rs780_vram_info(struct radeon_device *rdev)
{
	rs780_vram_get_type(rdev);

	/* FIXME: implement */
	/* Could aper size report 0 ? */
	rdev->mc.aper_base = drm_get_resource_start(rdev->ddev, 0);
	rdev->mc.aper_size = drm_get_resource_len(rdev->ddev, 0);
}
