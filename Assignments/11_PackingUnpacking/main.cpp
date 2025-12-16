#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <format>
#include <vector>

using namespace std;

constexpr size_t BUFFER_SIZE = 1 << 12; // 4KB 버퍼
constexpr int ENCODING = 0x00cd00ef;
const string PATH = "resources\\";
char fileData[BUFFER_SIZE];
streamsize bytesRead;

void unpacking(int index)
{
	char fileName[32];
	char* p = fileData + sizeof(ENCODING) + sizeof(int) + index * 40;
	memcpy(fileName, p, sizeof(fileName));
	p += sizeof(fileName);

	int size;
	memcpy(&size, p, sizeof(size));
	p += sizeof(size);

	int offset;
	memcpy(&offset, p, sizeof(offset));

	cout << format("fileName: {}, size: {}, offset: {}\n", fileName, size, offset);

	ofstream oFile(PATH + fileName, ios::trunc | ios::binary);
	oFile.write(fileData + offset, size);
}

int main()
{
	try
	{
		cout << "패킹하려면 1, 언패킹하려면 2를 쓰세요: ";

		int cmd;
		cin >> cmd;

		string fileName;
		int numberOfFile;

		if (cmd == 1)
		{
			cout << "패킹할 파일 개수를 쓰세요 (최대 5개): ";
			cin >> numberOfFile;

			vector<ifstream> packingFiles(numberOfFile);
			char packingFileNames[5][32];
			vector<int> fileSize(numberOfFile);

			for (int i = 0; i < numberOfFile; ++i)
			{
				cout << format("파일[{}] 이름 (최대 32 Byte): ", i);
				cin >> fileName;

				if (fileName.size() > 32)
				{
					cout << "파일명이 32 Byte보다 큽니다.\n";
					return 0;
				}

				packingFiles[i].open(PATH + fileName, ios::binary | ios::ate);
				if (!packingFiles[i])
					throw runtime_error("파일 열기 실패: " + fileName);

				fileSize[i] = packingFiles[i].tellg();
				cout << format("fileSize[{}]: {}\n", i, fileSize[i]);

				strcpy(packingFileNames[i], fileName.c_str());
			}

			cout << "패킹해서 최종적으로 생성할 파일명을 입력하세요: ";
			cin >> fileName;

			ofstream file(PATH + fileName, ios::binary | ios::trunc);
			if (!file)
				throw runtime_error("파일 열기 실패: " + fileName);

			file.write(reinterpret_cast<const char*>(&ENCODING), sizeof(ENCODING));
			file.write(reinterpret_cast<const char*>(&numberOfFile), sizeof(numberOfFile));

			int offset = sizeof(ENCODING) +
				sizeof(numberOfFile) +
				numberOfFile * (sizeof(packingFileNames[0]) + sizeof(fileSize[0]) + sizeof(int));

			for (int i = 0; i < numberOfFile; ++i)
			{
				file.write(packingFileNames[i], sizeof(packingFileNames[i]));
				file.write(reinterpret_cast<const char*>(&fileSize[i]), sizeof(fileSize[i]));
				cout << format("offset: {}\n", offset);
				file.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
				offset += fileSize[i];
			}

			for (int i = 0; i < numberOfFile; ++i)
			{
				packingFiles[i].seekg(0);
				packingFiles[i].read(fileData, fileSize[i]);
				file.write(fileData, fileSize[i]);
			}
		}
		else
		{
			cout << "언패킹할 파일명을 입력하세요: ";
			cin >> fileName;

			ifstream file(PATH + fileName, ios::binary);
			if (!file)
				throw runtime_error("파일 열기 실패: " + fileName);

			file.read(fileData, BUFFER_SIZE);
			bytesRead = file.gcount();

			int encoding;
			memcpy(&encoding, fileData, sizeof(ENCODING));

			if (encoding != ENCODING)
			{
				cout << "인코딩이 다릅니다. 올바르지 않은 파일이므로 종료합니다.\n";
				return 0;
			}

			memcpy(&numberOfFile, fileData + sizeof(ENCODING), sizeof(numberOfFile));

			cout << format("파일 개수는 {}개 입니다.\n", numberOfFile);
			cout << format("언패킹할 파일 번호 입력 (0 ~ {}) 또는 전체 언패킹은 {}: ", numberOfFile - 1, numberOfFile);
			cin >> cmd;

			if (cmd == numberOfFile)
				for (int i = 0; i < numberOfFile; ++i)
					unpacking(i);
			else if (0 <= cmd && cmd < numberOfFile)
				unpacking(cmd);
			else
				cout << "번호를 잘못 입력하셨습니다\n";
		}
	}
	catch (const exception e)
	{
		cerr << "에러: " << e.what() << '\n';
	}

	return 0;
}
