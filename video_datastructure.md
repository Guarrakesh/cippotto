(This document is AI-generated)
# CHIP-8 Video Buffer Data Structure

## Memory Layout Concept

```
CHIP-8 Display: 64x32 pixels
┌─────────────────────────────────────────┐
│ (0,0)  (1,0)  (2,0)  ...  (63,0)       │ ← Row 0 (top)
│ (0,1)  (1,1)  (2,1)  ...  (63,1)       │ ← Row 1
│ (0,2)  (1,2)  (2,2)  ...  (63,2)       │ ← Row 2
│  ...    ...    ...   ...   ...         │
│ (0,31) (1,31) (2,31) ... (63,31)       │ ← Row 31 (bottom)
└─────────────────────────────────────────┘
```

## Buffer Structure

```c
uint32_t video[64];  // 64 columns, each storing 32 pixels vertically
```

Each `video[x]` represents **one vertical column** of 32 pixels:

```
video[0] = Column 0:     video[1] = Column 1:     video[63] = Column 63:
┌─────────────────┐     ┌─────────────────┐      ┌─────────────────┐
│ bit 31 → (0,0)  │     │ bit 31 → (1,0)  │      │ bit 31 → (63,0) │
│ bit 30 → (0,1)  │     │ bit 30 → (1,1)  │  ... │ bit 30 → (63,1) │
│ bit 29 → (0,2)  │     │ bit 29 → (1,2)  │      │ bit 29 → (63,2) │
│     ...         │     │     ...         │      │     ...         │
│ bit 1  → (0,30) │     │ bit 1  → (1,30) │      │ bit 1  → (63,30)│
│ bit 0  → (0,31) │     │ bit 0  → (1,31) │      │ bit 0  → (63,31)│
└─────────────────┘     └─────────────────┘      └─────────────────┘
```

## Accessing Pixels

To access pixel at coordinate `(x, y)`:

**1. Find the column:** `video[x]`
**2. Find the bit position:** Depends on your bit ordering choice

### Option A: Bit 31 = Top (y=0), Bit 0 = Bottom (y=31)
```c
// To READ a pixel at (x,y):
uint8_t pixel = (video[x] >> (31 - y)) & 1;

// To SET a pixel at (x,y):
video[x] |= (1U << (31 - y));

// To CLEAR a pixel at (x,y):
video[x] &= ~(1U << (31 - y));
```

### Option B: Bit 0 = Top (y=0), Bit 31 = Bottom (y=31)
```c
// To READ a pixel at (x,y):
uint8_t pixel = (video[x] >> y) & 1;

// To SET a pixel at (x,y):
video[x] |= (1U << y);

// To CLEAR a pixel at (x,y):
video[x] &= ~(1U << y);
```

## Bitwise Operations Explained

### Setting a Bit (Turn pixel ON)
```c
video[x] |= (1U << bit_position);
```
- `1U << bit_position` creates a mask with only that bit set
- `|=` performs OR operation, turning that bit to 1 without affecting others

Example: Set pixel at (5, 10) using Option A:
```c
video[5] |= (1U << (31 - 10));  // Sets bit 21
```

### Clearing a Bit (Turn pixel OFF)
```c
video[x] &= ~(1U << bit_position);
```
- `1U << bit_position` creates a mask with that bit set
- `~` inverts it, so all bits are 1 except the target bit (which is 0)
- `&=` performs AND operation, turning that bit to 0 without affecting others

### Reading a Bit (Check if pixel is ON/OFF)
```c
uint8_t pixel = (video[x] >> bit_position) & 1;
```
- `>> bit_position` shifts the desired bit to position 0
- `& 1` masks off all other bits, leaving only the LSB

## Why This Approach?

1. **Memory Efficient:** 64 uint32_t = 256 bytes vs 2048 bytes for 2D array
2. **Fast Operations:** Can manipulate entire columns with single operations
3. **Cache Friendly:** Columns stored contiguously in memory
4. **Authentic:** Similar to how real hardware often works

## Implementation Consistency

Make sure your drawing code and display code use the same bit ordering convention:
- If drawing uses `video[x] |= (1U << y)` (Option B), then display should read with `(video[x] >> y) & 1`
- If drawing uses `video[x] |= (1U << (31 - y))` (Option A), then display should read with `(video[x] >> (31 - y)) & 1`

Mismatched conventions will cause vertically flipped sprites!
