# md5-dbb
Implementing dBB (den Boer–Bosselaers) style pseudo-MD5 hash collision.

--------

This implements a program that finds a MD5 dBB pseudo-collision, which is a collision for the compression function of MD5.
We refer to the paper "Collisions for the Compression Function of MD5" by Bert den Boer and Antoon Bosselaers for more information.

In detail: the program will find a block `M` such that for some IV value satisfying
```
(IV[1] & 0x80000000) == (IV[2] & 0x80000000)
(IV[1] & 0x80000000) == (IV[3] & 0x80000000)
```
then
```
G(IV, M) == G({
    IV[0] ^ 0x80000000,
    IV[1] ^ 0x80000000,
    IV[2] ^ 0x80000000,
    IV[3] ^ 0x80000000}, M)
```
where `G` is the compression function of MD5, as defined in den Boer and Bosselaers' paper.

## Compilation

First, you need to download https://github.com/cr-marcstevens/hashclash/blob/master/src/md5fastcoll/main.hpp and put it in the repository.

Then run `make`.

## Usage

Then, you need to find a first block that generates a dBB differential for the second block's IV value somehow.
For example, the paper "Fast Collision Attack on MD5" by Tao Xie, Fanbao Liu, Dengguo Feng contains a pair:

```
b505546ffe1e890b223515aeab41d53d08accf777770aeb479c74eb130cf7c0a5469c5f14533dc70a146da5e3017fcc9beb94809ad6cf72e6093148625cdce3b
b505546ffe1e890b223515aeab41d53d08accf777770aeb479c74eb130cf7c0a5469c5714533dc70a146da5e3017fcc9beb94809ad6cf72e6093148625cdce3b
```

By running the `md5-nopad` program, you get the IV values after running through the first block:

```
f334901e 5a01cce0 4b187535 25ca8c23
7334901e da01cce0 cb187535 a5ca8c23
```

You can then feed one of the two lines to `main` to get a block. An example output follows:

```
9f9ed23f187afc6f6f41c63a94e41692a3da4522ad3ebd459153cb4454c0efdce883ff23351bf7d5a7655e0e072c3349a0daf9d534acb88a4445363bcedb4453
```

You can verify the collision by running the following commands:

```bash
a=b505546ffe1e890b223515aeab41d53d08accf777770aeb479c74eb130cf7c0a5469c5f14533dc70a146da5e3017fcc9beb94809ad6cf72e6093148625cdce3b
b=b505546ffe1e890b223515aeab41d53d08accf777770aeb479c74eb130cf7c0a5469c5714533dc70a146da5e3017fcc9beb94809ad6cf72e6093148625cdce3b
c=9f9ed23f187afc6f6f41c63a94e41692a3da4522ad3ebd459153cb4454c0efdce883ff23351bf7d5a7655e0e072c3349a0daf9d534acb88a4445363bcedb4453
xxd -p -r <<< $a$c | md5sum
xxd -p -r <<< $b$c | md5sum
```

## Performance

In my testing, the program takes 10s to generate 100 collisions, which averages to 0.1s per collision.

I also modified Stevens' `fastcoll` (I use the version in https://github.com/brimstone/fastcoll although you can find the original source code in https://github.com/cr-marcstevens/hashclash/tree/master/src/md5fastcoll) to generate 100 collisions, and on a few test runs it took:
* 6s in `W` case
* 44s in `S00` case
* 18s in `S01` case
* 34s in `S10` case
* 38s in `S11` case

So in `W` case, Stevens' program is around the same speed as mine (possibly a bit faster).

You can use the following code to modify `fastcoll` to make it select `W` case and generate a lot of different block1s:

```diff
diff --git a/block0.cpp b/block0.cpp
index ad99358..6ba281f 100644
--- a/block0.cpp
+++ b/block0.cpp
@@ -288,7 +288,7 @@ void find_block0(uint32 block[], const uint32 IV[])
 					if ( (IHV3&(1<<25))!=0 || (IHV2&(1<<25))!=0 || (IHV1&(1<<25))!=0 
 						|| ((IHV2^IHV1)&1)!=0) stevens = false;
 										
-					if (!(wang || stevens)) continue;
+					if (!wang) continue;
 
 					std::cout << "." << std::flush;
 
diff --git a/main.cpp b/main.cpp
index b964560..e147577 100644
--- a/main.cpp
+++ b/main.cpp
@@ -43,6 +43,7 @@ notice and the version number should be present.
 */
 
 #include <iostream>
+#include <iomanip>
 #include <fstream>
 #include <time.h>
 
@@ -422,6 +423,7 @@ void find_collision(const uint32 IV[], uint32 msg1block0[], uint32 msg1block1[],
 	uint32 IHV[4] = { IV[0], IV[1], IV[2], IV[3] };
 	md5_compress(IHV, msg1block0);
 
+	while(true) {
 	if (verbose)
 		cout << endl << "Generating second block: " << flush;
 	find_block1(msg1block1, IHV);
@@ -435,4 +437,20 @@ void find_collision(const uint32 IV[], uint32 msg1block0[], uint32 msg1block1[],
 	msg2block1[4] += 1 << 31; msg2block1[11] -= 1 << 15; msg2block1[14] += 1 << 31;
 	if (verbose)
 		cout << endl;
+
+	for (int i = 0; i < 16; i++)
+		for (int j = 0; j < 4; j++)
+			cout<<hex<<setw(2)<<setfill('0')<<((msg1block0[i]>>(j*8))&0xFF);
+	for (int i = 0; i < 16; i++)
+		for (int j = 0; j < 4; j++)
+			cout<<hex<<setw(2)<<setfill('0')<<((msg1block1[i]>>(j*8))&0xFF);
+	cout<<'\n';
+	for (int i = 0; i < 16; i++)
+		for (int j = 0; j < 4; j++)
+			cout<<hex<<setw(2)<<setfill('0')<<((msg2block0[i]>>(j*8))&0xFF);
+	for (int i = 0; i < 16; i++)
+		for (int j = 0; j < 4; j++)
+			cout<<hex<<setw(2)<<setfill('0')<<((msg2block1[i]>>(j*8))&0xFF);
+	cout<<'\n'<<flush;
+	}
 }
```

## Algorithm

This does not follow the exact algorithm used by den Boer and Bosselaers.
Instead, it uses the Q9 tunnel, as described in the paper "Tunnels in Hash Functions: MD5 Collisions Within a Minute" by Vlastimil Klima.

## Acknowledgements

Thanks to MD5.1 challenge of BlueWater CTF 2024 to make me interested in MD5 collision attacks, at least for a few days. And of course Stevens for open sourcing his code, and the various researchers.

Side note, the authors of the paper "Fast Collision Attack on MD5" (Tao Xie, Fanbao Liu, Dengguo Feng) publishes their source code in [a book](https://www.amazon.com/Differential-Cryptanalysis-Hash-Functions-algorithms/dp/3659612006/). It is not open source however.
