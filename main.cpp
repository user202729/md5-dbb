#include "common.hpp"

int main(){
	array<uint32, 4> IV;

	for (int i = 0; i < 4; i++) {
		scanf("%8x", &IV[i]);
	}

	// given IV, find M such that if
	// Q0 = hash(M, IV)
	// Q1 = hash(M, IV + 0x80000000)
	// then (Q0[i] ^ Q1[i]) == 0x80000000 for all i
	array<uint32, 68> Q0_buffer { IV[0], IV[3], IV[2], IV[1] };
	let Q0 = Q0_buffer.begin() + Qoff;

	if (not (Q0[0] & 0x80000000)) {
		for(auto& x: IV) x ^= 0x80000000;
		Q0_buffer = { IV[0], IV[3], IV[2], IV[1] };
	}
	assert(Q0[-2] & 0x80000000);
	assert(Q0[-1] & 0x80000000);
	assert(Q0[0] & 0x80000000);

	array<uint32, 16> M {};

	while(true){
		// randomize everything
		for (int i = 1; i <= 16; i++) {
			Q0[i] = (
					i == 10 ? 0x80000000 :
					i == 11 ? 0xFFFFFFFF :
					(0xF8000000 | (xrng64() & 0x00FFFFFF)));
			modify_M(Q0, M, i);
		}
		if (not [&]{
			for (int i = 17; i <= 24; i++) {
				Q0[i] = compute_Q(Q0, M, i);
				if (not (Q0[i] & 0x80000000)) return false;
			}
			return true;
		}()) continue;

		for (Q0[9] = 0x80000000; Q0[9] < 0xFFFFFFFF; Q0[9]++) {
			// brute force Q9 tunnel
			modify_M(Q0, M, 9);
			modify_M(Q0, M, 10);
			modify_M(Q0, M, 13);
			for(int i = 1; i <= 24; i++)
				assert(Q0[i] == compute_Q(Q0, M, i));


			if (not [&]{ // around 2^21
				for(int i = 25; i <= 31; i++) {
					Q0[i] = compute_Q(Q0, M, i);
					if ((Q0[i] & 0x80000000) == 0) return false;
				}

				for(int i = 32; i <= 48; i++)
					Q0[i] = compute_Q(Q0, M, i);
				for(int i = 49; i <= 64; i++) {
					if ((Q0[i-1] ^ Q0[i-3]) & 0x80000000) return false;
					Q0[i] = compute_Q(Q0, M, i);
				}
				return true;
			}()) continue;

			// success

			{
				auto Q0_buffer_backup = Q0_buffer;
				array<uint32, 68> Q1_buffer { IV[0] ^ 0x80000000, IV[3] ^ 0x80000000, IV[2] ^ 0x80000000, IV[1] ^ 0x80000000 };
				let Q1 = Q1_buffer.begin() + Qoff;
				for (int i = 1; i <= 64; i++) {
					Q0[i] = compute_Q(Q0, M, i);
					Q1[i] = compute_Q(Q1, M, i);
				}
				//print_all(Q0, Q1, M, M);
				Q0_buffer = Q0_buffer_backup;
			}

			array<uint32, 68> Q1_buffer { IV[0] ^ 0x80000000, IV[3] ^ 0x80000000, IV[2] ^ 0x80000000, IV[1] ^ 0x80000000 };
			let Q1 = Q1_buffer.begin() + Qoff;
			for (int i = 1; i <= 64; i++) {
				Q1[i] = compute_Q(Q1, M, i);
				assert((Q0[i] ^ Q1[i]) == 0x80000000);
			}

			for (int i = 0; i < 16; i++) cout<<int_to_hex_little_endian(M[i]);
			cout<<'\n'<<flush;
		}
	}
}
