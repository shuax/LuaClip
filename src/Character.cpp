class UTF8FromUnicode
{
public:
	UTF8FromUnicode(const wchar_t *wszString)
	{
		length = ::WideCharToMultiByte(CP_UTF8, 0, wszString, -1, NULL, 0, NULL, NULL);
		szUTF8 = (char*)malloc(length * sizeof(char));
		::WideCharToMultiByte(CP_UTF8, 0, wszString, -1, szUTF8, length, NULL, NULL);
	}
	~UTF8FromUnicode() { free(szUTF8); }
	operator char*() const { return szUTF8; }
	int size() { return length * sizeof(char); }
	int len() { return length - 1; }
private:
	char* szUTF8;
	int length;
};

class AnsiFromUnicode
{
public:
	AnsiFromUnicode(const wchar_t *wszString)
	{
		length = ::WideCharToMultiByte(CP_ACP, 0, wszString, -1, NULL, 0, NULL, NULL);
		szAnsi = (char*)malloc(length * sizeof(char));
		::WideCharToMultiByte(CP_ACP, 0, wszString, -1, szAnsi, length, NULL, NULL);
	}
	~AnsiFromUnicode() { free(szAnsi); }
	operator char*() const { return szAnsi; }
	int size() { return length * sizeof(char); }
	int len() { return length - 1; }
private:
	char* szAnsi;
	int length;
};

class UnicodeFromUTF8
{
public:
	UnicodeFromUTF8(const char *szUTF8)
	{
		length = ::MultiByteToWideChar(CP_UTF8, 0, szUTF8, -1, NULL, 0);
		wszString = (wchar_t*)malloc(length * sizeof(wchar_t));
		::MultiByteToWideChar(CP_UTF8, 0, szUTF8, -1, wszString, length);
	}
	~UnicodeFromUTF8() { free(wszString); }
	operator wchar_t*() const { return wszString; }
	int size() { return length * sizeof(wchar_t); }
	int len() { return length - 1; }
private:
	wchar_t* wszString;
	int length;
};

class UnicodeFromAnsi
{
public:
	UnicodeFromAnsi(const char *szAnsi)
	{
		length = ::MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, NULL, 0);
		wszString = (wchar_t*)malloc(length * sizeof(wchar_t));
		::MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, wszString, length);
	}
	~UnicodeFromAnsi() { free(wszString); }
	operator wchar_t*() const { return wszString; }
	int size() { return length * sizeof(wchar_t); }
	int len() { return length - 1; }
private:
	wchar_t* wszString;
	int length;
};

#define AnsiFromUTF8(STR) AnsiFromUnicode(UnicodeFromUTF8((STR)))
#define UTF8FromAnsi(STR) UTF8FromUnicode(UnicodeFromAnsi((STR)))
