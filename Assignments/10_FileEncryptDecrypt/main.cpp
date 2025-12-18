#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <openssl/evp.h>

using namespace std;

constexpr size_t BUFFER_SIZE = 1 << 12; // 4KB 버퍼
constexpr int ENCODING = 0x00cd00ef;
const string PATH = "Resources\\";
char fileData[BUFFER_SIZE];
streamsize bytesRead;
string key;

vector<unsigned char> sha512(const vector<unsigned char>& input)
{
	EVP_MD_CTX* ctx = EVP_MD_CTX_new();
	if (!ctx)
		throw runtime_error("EVP_MD_CTX_new 실패");

	if (EVP_DigestInit_ex(ctx, EVP_sha512(), nullptr) != 1)
		throw runtime_error("EVP_DigestInit_ex 실패");

	if (EVP_DigestUpdate(ctx, input.data(), input.size()) != 1)
		throw runtime_error("EVP_DigestUpdate 실패");

	vector<unsigned char> hash(EVP_MAX_MD_SIZE);
	unsigned int hash_len = 0;

	if (EVP_DigestFinal_ex(ctx, hash.data(), &hash_len) != 1)
		throw runtime_error("EVP_DigestFinal_ex 실패");

	EVP_MD_CTX_free(ctx);
	hash.resize(hash_len);

	return hash;
}

vector<unsigned char> xorEncryptDecrypt(const vector<unsigned char>& input)
{
	vector<unsigned char> converted;
	size_t keyLen = key.size();

	for (size_t i = 0; i < input.size(); ++i)
		converted.emplace_back(input[i] ^ key[i % keyLen]);

	return converted;
}

int main()
{
	try
	{
		cout << "파일명을 입력하세요: ";

		string fileName;
		cin >> fileName;

		ifstream file(PATH + fileName, ios::in | ios::binary);
		if (!file)
			throw runtime_error("파일 열기 실패: " + fileName);

		file.read(fileData, BUFFER_SIZE);
		bytesRead = file.gcount();

		int encoding;
		memcpy(&encoding, fileData, sizeof(ENCODING));

		if (encoding == ENCODING) // encrypted file → original file
		{
			cout << "암호화된 파일입니다.\nkey를 입력하세요: ";
			cin >> key;

			unsigned char hash[EVP_MAX_MD_SIZE];
			memcpy(hash, fileData + sizeof(ENCODING), EVP_MAX_MD_SIZE);

			unsigned char encryptedData[BUFFER_SIZE];
			size_t size = bytesRead - (sizeof(ENCODING) + EVP_MAX_MD_SIZE);
			memcpy(encryptedData, fileData + sizeof(ENCODING) + EVP_MAX_MD_SIZE, size);

			vector<unsigned char> encryptedDataVector(size);
			for (int i = 0; i < size; ++i)
				encryptedDataVector[i] = encryptedData[i];

			vector<unsigned char> orgFileData = xorEncryptDecrypt(encryptedDataVector);
			vector<unsigned char> hashValue = sha512(orgFileData);

			if (!memcmp(hash, hashValue.data(), hashValue.size()))
			{
				cout << "key를 확인했습니다. 복호화를 수행합니다.\n";

				file.close();

				ofstream file(PATH + fileName, ios::out | ios::binary | ios::trunc);

				if (!file)
					throw runtime_error("파일 열기 실패: " + fileName);

				file.write(reinterpret_cast<const char*>(orgFileData.data()), orgFileData.size());
			}
			else
				cout << "key가 틀렸습니다.\n";

			file.close();
		}
		else // original file → encrypted file
		{
			cout << "일반 파일입니다. 암호화를 수행합니다.\n";

			vector<unsigned char>  fileDataVector(bytesRead);
			for (int i = 0; i < bytesRead; ++i)
				fileDataVector[i] = fileData[i];

			vector<unsigned char> hashValue = sha512(fileDataVector);
			file.close();

			ofstream file(PATH + fileName, ios::out | ios::binary | ios::trunc);

			if (!file)
				throw runtime_error("파일 열기 실패: " + fileName);

			file.write(reinterpret_cast<const char*>(&ENCODING), sizeof(ENCODING));
			file.write(reinterpret_cast<const char*>(hashValue.data()), hashValue.size());

			cout << "암호화할 key를 입력하세요: ";
			cin >> key;

			vector<unsigned char> converted = xorEncryptDecrypt(fileDataVector);
			file.write(reinterpret_cast<const char*>(converted.data()), converted.size());

			file.close();
		}
	}
	catch (const exception e)
	{
		cerr << "에러: " << e.what() << '\n';
	}

	return 0;
}
