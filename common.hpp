#if not LOCAL
#define NDEBUG 1
#endif

#include <iostream>
#include <iomanip>
#include <array>
#include <cassert>
#include <string>
#include <cstdio>
#include "main.hpp"  // https://github.com/cr-marcstevens/hashclash/blob/master/src/md5fastcoll/main.hpp

uint32 seed32_1 = uint32(time(NULL)), seed32_2 = 0x12345678; // must be nonzero otherwise random number generator will keep return 0

using namespace std;

#define let auto const

array<uint32, 64> const t = {0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x2441453,0xd8a1e681,0xe7d3fbc8,0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,0x289b7ec6,0xeaa127fa,0xd4ef3085,0x4881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391};
array<uint32, 64> const index = {
	0, 1, 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 14, 15,
	1, 6, 11, 0 , 5 , 10, 15, 4 , 9 , 14, 3 , 8 , 13, 2 , 7 , 12,
	5, 8, 11, 14, 1 , 4 , 7 , 10, 13, 0 , 3 , 6 , 9 , 12, 15, 2 ,
	0, 7, 14, 5 , 12, 3 , 10, 1 , 8 , 15, 6 , 13, 4 , 11, 2 , 9 ,
};
array<uint32 (*)(uint32, uint32, uint32), 64> const func = {
	FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, 
	GG, GG, GG, GG, GG, GG, GG, GG, GG, GG, GG, GG, GG, GG, GG, GG, 
	HH, HH, HH, HH, HH, HH, HH, HH, HH, HH, HH, HH, HH, HH, HH, HH, 
	II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, 
};
array<int, 64> const rotate_amounts = {
	7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
	 5, 9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
	4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
	6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
};

#define FOR_SUBSET(i, x) for(uint32 i = (x) + 1; i ? (i = (i - 1) & (x)), true : false;)

let F = [](auto& Q, int i){
	return func[i-1](Q[i-1], Q[i-2], Q[i-3]);
};
let W = [](auto& M, int i){
	return M[index[i-1]];
};
let compute_Q = [](auto& Q, auto& M, int i){
	return Q[i-1] + RL(Q[i-4] + F(Q, i) + W(M, i) + t[i-1], rotate_amounts[i-1]);
};
let back_compute_W = [](auto& Q, int i){
	return RL(Q[i] - Q[i-1], 32-rotate_amounts[i-1]) - t[i-1] - F(Q, i) - Q[i-4];
};
let modify_M = [](auto& Q, auto& M, int i){
	M[index[i-1]] = back_compute_W(Q, i);
};


let LIGHTBLACK_EX = "\x1b[90m";
let RESET = "\x1b[39m";

let int_to_hex = [](uint32 x) -> string {
	char buffer[9];
	sprintf(buffer, "%08x", x);
	string result;
	for(int i = 0; i < 8; i++){
		if (buffer[i] == '0') result += LIGHTBLACK_EX;
		result += buffer[i];
		if (buffer[i] == '0') result += RESET;
	}
	return result;
};

let int_to_hex_little_endian = [](uint32 x) -> string {
	char buffer[9];
	sprintf(buffer, "%08x", x);
	string result;
	for (int i = 6; i >= 0; i -= 2) {
		result += buffer[i];
		result += buffer[i + 1];
	}
	return result;
};

let print_all = [](auto& Q0, auto& Q1, auto& M0, auto& M1){
	cout<<
		string(9, ' ') +
		'q' + string(8, ' ') +
		'f' + string(8, ' ') +
		"w\n";

	for(int i = 1; i <= 32; i++){
		cout<<setw(3)<<i<<" | "
			<<int_to_hex(Q0[i] ^ Q1[i])<<' '
			<<int_to_hex(F(Q0, i) ^ F(Q1, i))<<' '
			<<int_to_hex(W(M0, i) ^ W(M1, i))<<" | ";
		int j = i + 32;
		cout
			<<int_to_hex(Q0[j] ^ Q1[j])<<' '
			<<int_to_hex(F(Q0, j) ^ F(Q1, j))<<' '
			<<int_to_hex(W(M0, j) ^ W(M1, j));
		cout<<'\n';
	}
};

