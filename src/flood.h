/*
 * Copyright (C) 2021 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
#define CONCATENATE(x, y) x ## y
#define CONCAT(x, y) CONCATENATE(x, y)

static SDL_Rect CONCAT(gfx_floodFill_, TYPE)(int x, int y, Uint32 col) {
	int old_color = *(TYPE *)PIXEL_AT(main_surface, x, y);
	if (old_color == col)
		return (SDL_Rect){};

	int minx = x, maxx = x, miny = y, maxy = y;

	int stack_size = 64;
	SDL_Point *stack = malloc(sizeof(SDL_Point) * stack_size);
	int top = 0;
	stack[top] = (SDL_Point){x, y};

	while (top >= 0) {
		x = stack[top].x;
		y = stack[top].y;
		top--;

		TYPE *line = PIXEL_AT(main_surface, 0, y);
		TYPE *prev_line = (TYPE *)((uint8_t *)line - main_surface->pitch);
		TYPE *next_line = (TYPE *)((uint8_t *)line + main_surface->pitch);
		while (x >= 0 && line[x] == old_color) x--;
		x++;
		minx = min(x, minx);

		bool span_above = false, span_below = false;
		for (; x < main_surface->w && line[x] == old_color; x++) {
			line[x] = col;
			if (y > 0) {
				if (!span_above && prev_line[x] == old_color) {
					if (++top >= stack_size) {
						stack_size *= 2;
						stack = realloc(stack, sizeof(SDL_Point) * stack_size);
					}
					stack[top] = (SDL_Point){x, y - 1};
					span_above = true;
				} else if (span_above && prev_line[x] != old_color) {
					span_above = false;
				}
			}
			if (y < main_surface->h - 1) {
				if (!span_below && next_line[x] == old_color) {
					if (++top >= stack_size) {
						stack_size *= 2;
						stack = realloc(stack, sizeof(SDL_Point) * stack_size);
					}
					stack[top] = (SDL_Point){x, y + 1};
					span_below = true;
				} else if (span_below && next_line[x] != old_color) {
					span_below = false;
				}
			}
		}
		maxx = max(x - 1, maxx);
		miny = min(y, miny);
		maxy = max(y, maxy);
	}
	free(stack);
	return (SDL_Rect){minx, miny, maxx - minx + 1, maxy - miny + 1};
}

#undef CONCATENATE
#undef CONCAT
