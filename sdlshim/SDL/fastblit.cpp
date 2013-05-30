
static FastBlit *convert_to_fastblit(uint16_t *image, int width, int height)
{
DBuffer code, pool;
FBGen fbgen;
int x, y, next_start_x;
int longest_run = 0;

	if (width == 0 || height == 0)
		return NULL;

	fbgen.code = &code;
	fbgen.pool = &pool;
	
	// initilize color register tracking
	for(int i=0;i<NUM_CREGS;i++)
	{
		fbgen.creg[i] = -1;
		fbgen.next_creg = 0;
	}

	write_code(0xE92D07F8, &fbgen);		// stmfd	sp!, {r2-r10}

	next_start_x = 0;
	for(y=0;y<height;y++)
	{
		// advance to next line
		if (y != 0)
		{
			write_code(0xE0800001, &fbgen);		// add r0, r0, r1
			image += width;
		}
		
		uint16_t *line = image;
		uint16_t *line_end = &image[width - 1];
		
		x = next_start_x;
		next_start_x = 0;
		
		while(x < width)
		{
			int color = line[x];
			int count = count_rle_length(color, &line[x], line_end);
			x += count;
			
			if (color == 0)
			{
				// coalesce empty runs at the end of this line with
				// runs at the beginning of the next line
				if (x == width && y+1 != height)
				{
					next_start_x = count_rle_length(0, image+width, line_end+width);
					count += next_start_x;
				}
				
				write_advance(count*2, &fbgen);
			}
			else
			{
				if (count > longest_run)
					longest_run = count;
				
				int creg = get_creg(color, true, true, &fbgen);
				uint32_t creg_mask = (creg << 12);
				
				// write code to draw color count times.
				// if count is over threshold we'll use a loop, else
				// we'll simply unroll the loop however many times.
				if (count >= FB_LOOP_THRESHOLD)
				{	// loopreg will be left at 0 when done so take that
					// into account just in case 0 is already a color
					int loopreg = get_creg(0, (color != 0), false, &fbgen);
					uint32_t loop_mask = (loopreg << 12) | (loopreg << 16);
					
					// now generate the code
					write_set_register(loopreg, count/2, &fbgen);	// mov rx, #count/2
					
					if (count & 1)
						write_code(0xE0C000B2|creg_mask, &fbgen);	// strh rx, [r0], #2
					
					write_code(0xE0C000B2|creg_mask, &fbgen);		// strh rx, [r0], #2
					write_code(0xE0C000B2|creg_mask, &fbgen);		// strh rx, [r0], #2
					
					write_code(0xE2500001|loop_mask, &fbgen);		// subs rx, rx, #1
					write_code(0x1AFFFFFB, &fbgen);					// bne loop
				}
				else
				{
					while(count)
					{
						write_code(0xE0C000B2|creg_mask, &fbgen);	// strh rx, [r0], #2
						count--;
					}
				}
			}
		}
	}

	write_code(0xE8BD07F8, &fbgen);		// ldmfd	sp!, {r3-r10}
	write_code(0xE12FFF1E, &fbgen);		// bx lr
	
	stat("\nLongest run %d bytes", longest_run);
	stat("Pool length %04x bytes", pool.Length());
	
	if (pool.Length() > 0xff)
	{
		stat("ERROR: pool length > 255 bytes (0x%04x)", pool.Length());
		return NULL;
	}
	
	// first thing to do on function-entry will be to
	// load offset of pool using PC-relative addressing.
	// add 8 because of the +8 the PC is read as
	DBuffer entry;
	int pool_offset = pool.Length() + 8;
	
	int amt = min(pool_offset, 0xff);
	entry.Append32(0xE24F2000 | amt);		// sub r2, pc, #xx
	pool_offset -= amt;
	
	while(pool_offset)
	{
		int amt = min(pool_offset, 0xff);
		entry.Append32(0xE2422000 | amt);	// sub r2, r2, #xx
		pool_offset -= amt;
	}

	// combine code and pool
	int buffer_length = pool.Length() + entry.Length() + code.Length() + 4;
	uint8_t *buffer = (uint8_t *)malloc(buffer_length);
	uint8_t *ptr = buffer;
	
	// DWORD-align
	while((uint32_t)ptr & 3) ptr++;
	
	// combine all the pieces while obtaining the entry point in ptr
	memcpy(ptr, pool.Data(), pool.Length());
	ptr += pool.Length();
	
	memcpy(ptr, entry.Data(), entry.Length());
	memcpy(ptr+entry.Length(), code.Data(), code.Length());
	
	// done. now return the result.
	FastBlit *fb = new FastBlit;
	
	fb->entry_point = (FastBlitFunction)ptr;
	fb->buffer = buffer;
	fb->length = buffer_length;

	return fb;
}


static int count_rle_length(uint16_t color, uint16_t *ptr, uint16_t *maxend)
{
	uint16_t *start = ptr;
	while(ptr <= maxend && *ptr == color)
		ptr++;
	
	return (ptr - start);
}


// return the number of a creg holding the given color.
// if no cregs currently hold the given color, one is kicked out
// and loaded with the given color and it's index returned.
static int get_creg(uint16_t color, bool reuse_old, bool generate_code, FBGen *fbgen)
{
	if (reuse_old)
	{
		for(int i=0;i<NUM_CREGS;i++)
		{
			if (fbgen->creg[i] == color)
				return (i + FIRST_CREG);
		}
	}

	fbgen->creg[fbgen->next_creg] = color;
	
	int regno = (fbgen->next_creg + FIRST_CREG);
	if (++fbgen->next_creg >= NUM_CREGS)
		fbgen->next_creg = 0;
	
	if (generate_code)
	{
		write_set_register(regno, color, fbgen);
	}
	
	return regno;
}

/*
void c------------------------------() {}
*/

// advance the pointer by "amt" bytes
static void write_advance(int amt, FBGen *fbgen)
{
	while(amt > 255)
	{
		write_code(0xE28000FF, fbgen);		// add r0, r0, #0xff
		amt -= 255;
	}
	
	uint32_t opcode = 0xE2800000;	// add r0, r0, #xx
	write_code(opcode | amt, fbgen);
}

// write code to set register regno to newvalue
static void write_set_register(int regno, uint32_t newvalue, FBGen *fbgen)
{
uint32_t opcode;

	if (newvalue <= 0xff)
	{
		opcode = 0xE3A00000;	// mov rx, #xx
		opcode |= (regno << 12);
		opcode |= newvalue;
		write_code(opcode, fbgen);
	}
	else
	{
		opcode = 0xE5920000;	// ldr rx, [r2, #xx]
		opcode |= (regno << 12);
		opcode |= write_pool(newvalue, fbgen);
		write_code(opcode, fbgen);
	}
}

// add the given opcode to the code section
static void write_code(uint32_t opcode, FBGen *fbgen)
{
	fbgen->code->Append32(opcode);
}

// add the given value to the pool and return it's offset within the pool
static int write_pool(uint32_t value, FBGen *fbgen)
{
	// check if already in pool
	uint8_t *pool = (uint8_t *)fbgen->pool->Data();
	int length = fbgen->pool->Length();
	
	for(int i=0;i<length;i+=4)
	{
		// can't read as a uint32 because it may not be aligned
		// (stupid memory manager)
		uint32_t pv = ((uint32_t)pool[i+3] << 24 | \
						(uint32_t)pool[i+2] << 16 | \
						(uint32_t)pool[i+1] << 8 | \
						(uint32_t)pool[i+0]);
		
		if (pv == value)
		{
			return i;
		}
	}

	fbgen->pool->Append32(value);
	return (fbgen->pool->Length() - 4);
}
