/*
 * camera.hpp
 * Copyright(c) 2021 by Caz Yokoyama, caz@caztech.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "ESPAsyncWebServer.h"
#include "dl_lib_matrix3d.h"

void camera_init();
dl_matrix3du_t *acquire_rgb888();
void release_rgb888(dl_matrix3du_t *image_matrix);
uc_t get_average_brightness(dl_matrix3du_t *image_matrix);
void capturePhoto(uint8_t **_jpg_buf, size_t *_jpg_buf_len);

#endif /* CAMERA_HPP */
