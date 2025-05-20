# LEA - the Lowery Encryption Algorithm

I pronounce it lay·uh (like Princess Leia), but pronounce it however you like, this isn't GNU.
There are two main components to it, the **ECHO** hashing algorithm, and the **BSC** symmetric encryption algorithm.
Both of these components are built and designed by me.

---

## ECHO - Expand Compact Hashing Order

ECHO uses two key steps in its hashing process: **Bit Interleaving Expansion (BIE)** and **Sequential Bit Compaction (SBC)**. These work together to transform a password into a 256-bit keyhash with strong diffusion, ensuring similar inputs produce very different outputs.

### Bit Interleaving Expansion (BIE)

**What it does**: Expands a 256-bit input into 512 bits by mixing original bits with transformed, input-derived bits.

**How it works**:

1. Split the input into 32 bytes (8 bits each). If input is shorter, loop over it.
2. For each byte (e.g., `01000001` for "A"):
   - Multiply its value by a prime (e.g., 17), take the 8-bit result (e.g., `01010001`).
   - Flip those bits (e.g., `10101110`).
3. Interleave: Place original bits (reversed) at even indices (0, 2, ...), transformed bits at odd indices (1, 3, ...).

**Example**:

- Input: `01000001`.
- Byte: `01000001` → `* 17 = 1105` (truncate bits after 8) → `01010001` → flip to `10101110`.
- Interleave reversed input (`10000010`) with transformed byte (`10101110`): `1100010001011100`.
- Output: 16 bits, half original, half scrambled.

**Why**: Doubles the data with chaos, making small changes (e.g., `0100` to `0101`) spread wide.

### Sequential Bit Compaction (SBC)

**What it does**: Compresses the 512-bit BIE output back to 256 bits while mixing bits further.

**How it works**:

1. Take the 512-bit input.
2. XOR each pair of adjacent bits (0 with 1, 2 with 3, etc.) to produce 256 output bits.

**Example**:

- Input: `01 00 11 01` (8 bits, imagine 512 total).
- XOR pairs: `0^1=1`, `0^0=0`, `1^1=0`, `0^1=1`.
- Output: `1001` (4 bits, scales to 256 in full).
- Full run: Each of 256 output bits combines an original and transformed bit.

**Why**: Folds the expanded data, ensuring every output bit reflects both input and its derived chaos.

### Together

- **BIE**: Spreads 256 bits to 512 with interleaved transforms.
- **SBC**: Compacts 512 back to 256, mixing original and derived bits.
- Result: A 256-bit hash where tiny input changes (e.g., "abc" vs "abd") flip many output bits, proven by testing 10M+ hashes to ensure **Collision Resistance** and **Avalanche Effect** achievement.

More rounds and tweaks (rotation, flips) build on this core for extra strength.
