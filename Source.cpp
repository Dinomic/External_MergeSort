#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <string>
using namespace std;


string int2str(int input)
{
	vector<char> char_arr = { '0','1','2','3','4','5','6','7','8','9' };

	string output;

	if (input == 0)
		return "0";

	while (input != 0)
	{
		int x = input % 10;
		output.push_back(char_arr[x]);
		input /= 10;
	}
	reverse(output.begin(), output.end());
	return output;
}
double Random(double n)
{
	return n * rand() / RAND_MAX;
}
void create_data()
{
	for (int i = 1; i <= 10; i++)
	{
		string datapath = "";
		datapath.append(int2str(i));
		datapath.append("00M.bin");

		double* buffer = new double[64];

		ofstream output;
		output.open(datapath, ios::out | ios::app | ios::ate | ios::binary);

		//ghi du lieu
		long j = 0;
		while (j < i * 100000000)
		{
			int size = 0;
			for (int k = 0; k < 64; k++)
			{
				if (j < i * 100000000)
				{
					buffer[k] = Random(100000);
					j++;
					size++;
				}
			}
			output.write((char*)buffer, sizeof(double) * size);
		}

		output.close();
	}
}

void merge_turn(fstream *f, fstream *g, vector<string> f_name, vector<string> g_name, long &n) /// f -> d
{
	//cout << "merge turn" << endl;
	const int BSize = 512;
	const int BEleNumber = 512 / sizeof(double);

	double** readB = new double*[n];
	long* readBIndex = new long[n];

	long *BNumber = new long[n];
	long *LBSize = new long[n];
	long *currentB = new long[n];

	long* currentRun = new long[n];

	long realRun = -1;

	double* writeB = new double[BSize];
	long writeBIndex = 0;

	long writeFIndex = 0;

	long writeDone = 0;
	long *writeIsDone = new long[n];

	long *checkRun = new long[n];
	double EndREle = 0.0;

	double maxRun;


	for (int i = 0; i < n; i++) /// duyet file f
	{
		readB[i] = new double[BSize];
		readBIndex[i] = 0;

		f[i].open(f_name[i], ios::in | ios::binary);
		f[i].seekg(0, ios::end);
		streampos end = f[i].tellg();
		BNumber[i] = end / BSize;
		LBSize[i] = end % BSize;

		if (i == 0)
		{
			f[i].seekg(-8, ios::end);
			f[i].read((char*)&maxRun, sizeof(double));
		}


		f[i].seekg(0, ios::beg);
		currentB[i] = 1;

		currentRun[i] = -1;
		f[i].seekg(0, ios::beg);

		writeIsDone[i] = 0;

		checkRun[i] = 0;
	}
	//cout << maxRun << endl;

	for (int i = 0; i < n; i++)
	{
		g[i].open(g_name[i], ios::out | ios::trunc | ios::binary);
	}

	for (int i = 0; i < n; i++) /// doc buffer dau tien
	{

		if (BNumber[i] > 0)
		{
			f[i].read((char*)readB[i], BSize);
		}
		else
		{
			if (LBSize[i] > 0)
			{
				f[i].read((char*)readB[i], LBSize[i]);
			}
		}
	}

	/// cac lan sau

	while (writeDone < n)
	{
		//cout << " loi 1" << endl;
		long min = 1e9;
		long thechosenB = -1;
		long thechosenBI = -1;
		for (int i = 0; i < n; i++)
		{


			if (writeIsDone[i])
			{
				continue;
			}

			if (checkRun[i] == 1)
			{
				continue;
			}


			if (LBSize[i] == 0) // khong thua 
			{
				if (readBIndex[i] == BEleNumber) // da ghi het buffer
				{
					if (currentB[i] == BNumber[i]) // dang o buffer cuoi
					{
						//cout << "loi 1.1" << endl;
						writeIsDone[i] = 1;
						writeDone++;
						continue;
					}
					else // dang o buffer thuong -> ghi them vao buffer
					{
						f[i].read((char*)readB[i], BSize);
						currentB[i]++;
						readBIndex[i] = 0;
					}
				}
			}
			else // co thua
			{
				if (readBIndex[i] == (LBSize[i] / sizeof(double)) && currentB[i] > BNumber[i]) //// ghi het buffer cuoi
				{
					//cout << "loi 1.2" << endl;
					writeIsDone[i] = 1;
					writeDone++;
					continue;
				}
				if (readBIndex[i] == BEleNumber) //// ghi het buffer thuong
				{
					if (currentB[i] == BNumber[i]) /// ke cuoi
					{
						f[i].read((char*)readB[i], LBSize[i]);
						currentB[i]++;
						readBIndex[i] = 0;
					}
					if (currentB[i] < BNumber[i]) /// real thuong
					{
						f[i].read((char*)readB[i], BSize);
						currentB[i]++;
						readBIndex[i] = 0;
					}
				}
			}


			if (readB[i][readBIndex[i]] == realRun)
			{
				checkRun[i] = 1;
				readBIndex[i]++;
			}
			else if (readB[i][readBIndex[i]] < min)
			{
				if (min != 1e9)
				{
					readBIndex[thechosenB]--;
					min = readB[i][readBIndex[i]];
					thechosenB = i;
					thechosenBI = readBIndex[i];
					readBIndex[i]++;
				}
				else
				{
					min = readB[i][readBIndex[i]];
					thechosenB = i;
					thechosenBI = readBIndex[i];
					readBIndex[i]++;
				}
			}
		}

		if (thechosenB == -1) /// ko tim dc min -> het run
		{
			g[writeFIndex % n].write((char*)writeB, writeBIndex * sizeof(double));//
			if (writeFIndex % n == 0)
				EndREle--;
			g[writeFIndex % n].write((char*)&EndREle, sizeof(double));
			writeFIndex++;
			writeBIndex = 0;

			if (realRun == maxRun)
				break;

			realRun--;
			for (int i = 0; i < n; i++)
			{
				checkRun[i] = 0;
			}
		}
		else /// tim dc min
		{
			if (writeBIndex == BEleNumber) /// het buffer
			{
				g[writeFIndex % n].write((char*)writeB, writeBIndex * sizeof(double));
				writeBIndex = 0;
				writeB[writeBIndex] = readB[thechosenB][thechosenBI];
				writeBIndex++;
			}
			else /// con buffer
			{
				writeB[writeBIndex] = readB[thechosenB][thechosenBI];
				writeBIndex++;
			}
		}
	}

	for (int i = 0; i < n; i++)
	{
		f[i].close();
		g[i].close();
	}

	if (writeFIndex < n)
		n = writeFIndex;
	//cout << n << "merge" << endl;
}


void sort(int n, string datapath)
{
	const int BSize = 512;
	const int BEleNumber = 512 / sizeof(double);

	ifstream data;
	data.open(datapath, ios::in | ios::binary);

	fstream *f = new fstream[n];
	vector<string> f_name(n, "f");
	for (int i = 0; i < n; i++)
	{
		f_name[i].append(int2str(i));
		f_name[i].append(".bin");

		f[i].open(f_name[i], ios::app | ios::binary);
	}

	fstream *g = new fstream[n];
	vector<string> g_name(n, "g");
	for (int i = 0; i < n; i++)
	{
		g_name[i].append(int2str(i));
		g_name[i].append(".bin");

		g[i].open(g_name[i], ios::app | ios::binary);
	}

	for (int i = 0; i < n; i++)
	{
		g[i].close();
		f[i].close();
	}

	// read from data.bin and write to N f.bin files

	data.seekg(0, ios::end);
	streampos end = data.tellg();
	long BNumber = end / BSize;
	long LastBSize = end % BSize;
	data.seekg(0, ios::beg);

	for (int i = 0; i < n; i++)
	{
		f[i].open(f_name[i], ios::out | ios::trunc | ios::binary);
	}

	double buffer[BSize / sizeof(double)];
	double Sbuffer[BSize / sizeof(double)];

	double endREle = 0.0;
	double LastBEle;
	long fileNumber = n;
	long FIndex = 0;
	int SBIndex = 0;

	if (BNumber > 0)
	{
		for (int i = 0; i < BNumber; i++)
		{
			data.read((char*)buffer, BSize);
			if (i > 0 && buffer[0] < LastBEle)
			{
				if (FIndex % n == 0) endREle--;
				f[FIndex % n].write((char*)&endREle, sizeof(double));
				FIndex++;
			}
			for (int j = 0; j < BEleNumber - 1; j++)
			{
				if (buffer[j] > buffer[j + 1])
				{
					Sbuffer[SBIndex] = buffer[j];
					SBIndex++;
					f[FIndex % n].write((char*)Sbuffer, SBIndex * sizeof(double));
					if (FIndex % n == 0) endREle--;
					f[FIndex % n].write((char*)&endREle, sizeof(double));
					FIndex++;
					SBIndex = 0;
				}
				else
				{
					Sbuffer[SBIndex] = buffer[j];
					SBIndex++;
				}
			}
			LastBEle = buffer[BEleNumber - 1];
			Sbuffer[SBIndex] = buffer[BEleNumber - 1];
			SBIndex++;
			f[FIndex % n].write((char*)Sbuffer, SBIndex * sizeof(double));
			SBIndex = 0;
		}
		if (LastBSize > 0)
		{
			SBIndex = 0;
			data.read((char*)buffer, LastBSize);
			if (buffer[0] < LastBEle)
			{
				if (FIndex % n == 0) endREle--;
				f[FIndex % n].write((char*)&endREle, sizeof(double));
				FIndex++;
			}
			for (int j = 0; j < LastBSize / sizeof(double) - 1; j++)
			{
				if (buffer[j] > buffer[j + 1])
				{
					Sbuffer[SBIndex] = buffer[j];
					SBIndex++;
					f[FIndex % n].write((char*)Sbuffer, SBIndex * sizeof(double));
					if (FIndex % n == 0) endREle--;
					f[FIndex % n].write((char*)&endREle, sizeof(double));
					FIndex++;
					SBIndex = 0;
				}
				else
				{
					Sbuffer[SBIndex] = buffer[j];
					SBIndex++;
				}
			}

			Sbuffer[SBIndex] = buffer[LastBSize / sizeof(double) - 1];
			SBIndex++;
			f[FIndex % n].write((char*)Sbuffer, SBIndex * sizeof(double));
			SBIndex = 0;
			if (FIndex % n == 0) endREle--;
			f[FIndex % n].write((char*)&endREle, sizeof(double));
			FIndex++;
		}
		else
		{
			if (FIndex % n == 0) endREle--;
			f[FIndex % n].write((char*)&endREle, sizeof(double));
			FIndex++;
		}
	}
	else
	{
		if (LastBSize > 0)
		{
			SBIndex = 0;
			data.read((char*)buffer, LastBSize);
			for (int j = 0; j < LastBSize / sizeof(double) - 1; j++)
			{
				if (buffer[j] > buffer[j + 1])
				{
					Sbuffer[SBIndex] = buffer[j];
					SBIndex++;
					f[FIndex % n].write((char*)Sbuffer, SBIndex * sizeof(double));
					if (FIndex % n == 0) endREle--;
					f[FIndex % n].write((char*)&endREle, sizeof(double));
					FIndex++;
					SBIndex = 0;
				}
				else
				{
					Sbuffer[SBIndex] = buffer[j];
					SBIndex++;
				}
			}
			Sbuffer[SBIndex] = buffer[LastBSize / sizeof(double) - 1];
			SBIndex++;
			f[FIndex % n].write((char*)Sbuffer, SBIndex * sizeof(double));
			SBIndex = 0;
			if (FIndex % n == 0) endREle--;
			f[FIndex % n].write((char*)&endREle, sizeof(double));
			FIndex++;
		}
	}
	data.close();
	for (int i = 0; i < n; i++)
	{
		f[i].close();
	}

	if (FIndex > n - 1)
		fileNumber = n;
	else
		fileNumber = FIndex;

	//cout << fileNumber << endl;


	long count = 0;
	while (fileNumber > 1)
	{
		//cout << "loi 2" << endl;
		if (count % 2 == 0)
		{
			/// f -> d
			merge_turn(f, g, f_name, g_name, fileNumber);
			count++;
			//cout << count << endl;

		}
		else if (count % 2 == 1)
		{
			/// d -> f
			merge_turn(g, f, g_name, f_name, fileNumber);
			count++;
			//cout << count << endl;
		}
	}

	double *BB = new double[BSize];
	string outputname = datapath;
	outputname.append("_");
	outputname.append(int2str(n));
	outputname.append("_way_sorted.bin");
	fstream output;
	output.open(outputname, ios::out | ios::binary | ios::trunc);

	if (count % 2 == 0)
	{
		f[0].open(f_name[0], ios::in | ios::binary);

		f[0].seekg(0, ios::end);
		streampos end = f[0].tellg() - (streampos)sizeof(double);
		f[0].seekg(0, ios::beg);

		long soB = end / BSize;
		long sizeBcuoi = end % BSize;

		for (int i = 0; i < soB; i++)
		{
			f[0].read((char*)BB, BSize);
			output.write((char*)BB, BSize);
		}
		if (sizeBcuoi > 0)
		{
			f[0].read((char*)BB, sizeBcuoi);
			output.write((char*)BB, sizeBcuoi);
		}

		f[0].close();
	}
	else if (count % 2 == 1)
	{
		g[0].open(g_name[0], ios::in | ios::binary);

		g[0].seekg(0, ios::end);
		streampos end = g[0].tellg() - (streampos)sizeof(double);
		g[0].seekg(0, ios::beg);

		long soB = end / BSize;
		long sizeBcuoi = end % BSize;

		for (int i = 0; i < soB; i++)
		{
			g[0].read((char*)BB, BSize);
			output.write((char*)BB, BSize);
		}
		if (sizeBcuoi > 0)
		{
			g[0].read((char*)BB, sizeBcuoi);
			output.write((char*)BB, sizeBcuoi);
		}

		g[0].close();
	}

	output.close();
}

long test()
{
	const int BSize = 512;
	const int BEleNumber = 512 / sizeof(double);

	double buffer1[BSize / sizeof(double)];
	string output_file;
	cout << endl << "Note: formal name of sorted file: k_way_sorted_SourceFileName.bin" << endl;
	cout << "Example: 100_way_sorted_10_8.bin" << endl;
	cout << "You should find the file name in project folder" << endl;
	cout << "to make sure the file to be checked was the output file. " << endl;
	cout << "Please input the file name need to be checked:" << endl;
	cin >> output_file;
	ifstream out(output_file, ios::in | ios::binary);
	if (!out.is_open())
	{
		cout << "Can't open the file!";
		return 0;
	}
	out.seekg(0, ios::end);
	long k = out.tellg();
	long m = k / BSize;
	long r = k % BSize;
	double end_buf_ele = -1;
	out.seekg(0, ios::beg);
	if (m > 0)
	{
		for (long j = 0; j < m; j++)
		{
			out.read(reinterpret_cast<char*>(buffer1), BSize);
			if (buffer1[0] < end_buf_ele)
			{
				cout << endl << "Wrong1!" << endl;
				cout << buffer1[0] << " " << end_buf_ele << endl;
				out.close();
				return 0;
			}
			for (long a = 0; a < BEleNumber - 1; a++)
				if (buffer1[a] > buffer1[a + 1])
				{
					cout << endl << "Wrong2!" << endl;
					cout << buffer1[a] << " " << buffer1[a + 1] << endl;
					out.close();
					return 0;
				}
			end_buf_ele = buffer1[BEleNumber - 1];
		}
		if (r > 0)
		{
			out.read(reinterpret_cast<char*>(buffer1), r);
			if (buffer1[0] < end_buf_ele)
			{
				cout << endl << "Wrong3!" << endl;
				cout << buffer1[0] << " " << end_buf_ele << endl;
				out.close();
				return 0;
			}
			for (long a = 0; a < (r / sizeof(double)) - 1; a++)
				if (buffer1[a] > buffer1[a + 1])
				{
					cout << endl << "Wrong4!" << endl;
					cout << buffer1[a] << " " << buffer1[a + 1] << endl;
					out.close();
					return 0;
				}
		}
	}
	else
	{
		out.read(reinterpret_cast<char*>(buffer1), r);
		for (long a = 0; a < (r / sizeof(double)) - 1; a++)
			if (buffer1[a] > buffer1[a + 1])
			{
				cout << endl << "Wrong5!" << endl;
				cout << buffer1[a] << " " << buffer1[a + 1] << endl;
				out.close();
				return 0;
			}
	}
	out.close();
	cout << endl << "Right!" << endl;
	return 1;
}




int main()
{
	//create_data();

	string datapath = "300M.bin";
	cout << datapath << endl;
	for (int i = 0; i < 9; i++)
	{
		int nway;
		clock_t begin_time = clock();
		//if (i == 0)
		//{
		//	nway = 2;
		//	sort(nway, datapath);
		//	clock_t end_time = clock();
		//	cout << endl << "Time to sort file( " << nway << " ): " << (double)(end_time - begin_time) / (double)CLOCKS_PER_SEC << endl;
		//}
		if (i == 1)
		{
			nway = 10;
			sort(nway, datapath);
			clock_t end_time = clock();
			cout << endl << "Time to sort file( " << nway << " ): " << (double)(end_time - begin_time) / (double)CLOCKS_PER_SEC << endl;
		}
		//else if (i == 2)
		//{
		//	nway = 20;
		//	sort(nway, datapath);
		//}
		//else if (i == 3)
		//{
		//	nway = 35;
		//	sort(nway, datapath);
		//}
		else if (i == 4)
		{
			nway = 50;
			sort(nway, datapath);
			clock_t end_time = clock();
			cout << endl << "Time to sort file( " << nway << " ): " << (double)(end_time - begin_time) / (double)CLOCKS_PER_SEC << endl;
		}
		//else if (i == 5)
		//{
		//	nway = 65;
		//	sort(nway, datapath);
		//}
		//else if (i == 6)
		//{
		//	nway = 80;
		//	sort(nway, datapath);
		//}
		//else if (i == 7)
		//{
		//	nway = 90;
		//	sort(nway, datapath);
		//}
		else if (i == 8)
		{
			nway = 100;
			sort(nway, datapath);
			clock_t end_time = clock();
			cout << endl << "Time to sort file( " << nway << " ): " << (double)(end_time - begin_time) / (double)CLOCKS_PER_SEC << endl;
		}
		//clock_t end_time = clock();
		//cout << endl << "Time to sort file( " << nway << " ): " << (double)(end_time - begin_time) / (double)CLOCKS_PER_SEC << endl;
	}

	//test();

	//srand(time(NULL));
	//double* a = new double[10000];

	//ofstream output("data.bin", ios::out | ios::trunc | ios::binary);

	//for (int i = 0; i < 10000; i++)
	//{
	//	a[i] = Random(1000);
	//}

	//output.write((char*)a, sizeof(double) * 10000);
	//output.close();

	//sort(4, "data.bin");

	//ifstream f("data.bin_4_way_sorted.bin", ios::in | ios::binary);
	//
	//double* b = new double[128];
	//

	//f.seekg(0, ios::end);
	//long end = f.tellg();
	//f.seekg(0, ios::beg);
	//
	//long Num = end / (128 * sizeof(double));
	//long LBS = end % (128 * sizeof(double));

	//long long c = 0;

	//for (int i = 0; i < Num; i++)
	//{
	//	f.read((char*)b, 128 * 8);
	//	for (int j = 0; j < 128; j++)
	//	{
	//		cout << b[j] << endl;
	//	}
	//}
	//if (LBS > 0)
	//{
	//	f.read((char*)b, LBS);
	//	for (int i = 0; i < LBS/8; i++)
	//	{
	//		cout << b[i] << endl;
	//	}
	//}

	system("pause");
	return 0;
}