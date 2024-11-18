#include "common.hpp"

int main(){
	array<uint32, 4> const IV = {0x67452301,0xefcdab89,0x98badcfe,0x10325476};
	array<uint32, 16> M;

	for (auto& x: M) {
		scanf("%8x", &x);
		x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
	}

	array<uint32, 68> Q_buffer { IV[0], IV[3], IV[2], IV[1] };
	let Q = Q_buffer.begin() + Qoff;

	for (int i = 1; i <= 64; i++) {
		Q[i] = compute_Q(Q, M, i);
	}

	array<uint32, 4> IHV = {
		IV[0] + Q[61],
		IV[1] + Q[64],
		IV[2] + Q[63],
		IV[3] + Q[62],
	};

	for(auto x: IHV) printf("%08x ", x);
	printf("\n");
}
