/*
 * EnvInterface64Impl.cpp
 *
 *  Created on: Nov 25, 2017
 *      Author: 13774
 */

// 64--ONLY

#include <EnvInterface64Impl.h>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <regex>
#include <utility>

#if defined(CODE64)



EnvInterface64Impl::EnvInterface64Impl()
{

}

u8_t EnvInterface64Impl::addFile(const std::string & file,u32_t lbaBase)
{
	u8_t driver;
	if( (driver=getFileDriver(file))!=0xff)return driver;
	driver =files.size();
	if(driver!=0xff)
	{
		files.push_back(std::make_pair(file,lbaBase));
	}
	return driver;
}
u8_t EnvInterface64Impl::getFileDriver(const std::string & file)const
{
	for(size_t i=0;i!=files.size();++i)
		if(files[i].first == file)
			return i;
	return 0xff;
}
const std::string & EnvInterface64Impl::getDriverFile(u8_t driver)const
{
	static const std::string nullstr="";
	if(driver < files.size())
		return files.at(driver).first;
	return nullstr;
}

int EnvInterface64Impl::writeSectors(u32_t srcSeg,const u8_t* srcOff,u8_t driver,u32_t LBAlow,u32_t num,u32_t LBAhigh)
{
	const std::string &file=getDriverFile(driver);
	if(file!="")
	{
		u32_t lbaBase = files[driver].second;
		FILE *fp=fopen(file.c_str(),"rb+");
		if(fp==NULL)
			fp=fopen(file.c_str(),"wb+");
		fseek(fp,(LBAlow - lbaBase)*CONST_SECSIZE,SEEK_SET);
		size_t n=fwrite(srcOff,CONST_SECSIZE,num,fp);
		fclose(fp);
		return n;
	}else{
		return 0;
	}

}
int EnvInterface64Impl::readSectors(u32_t dstSeg,u8_t* dstOff,u8_t driver,u32_t LBAlow,u32_t num,u32_t LBAhigh)
{
	const std::string &file=getDriverFile(driver);
	if(file!="")
	{
		u32_t lbaBase = files[driver].second;
		FILE *fp=fopen(file.c_str(),"rb+");
			if(fp==NULL)
				fp=fopen(file.c_str(),"wb+");
		//	fseek(fp,0,SEEK_END);
		//	printf_simple("fp size=%d\n",ftell(fp));
			fseek(fp,(LBAlow - lbaBase)*CONST_SECSIZE,SEEK_SET); // 如果已有文件不够大，会返回错误

		//	size_t n=fread(dstOff,CONST_SECSIZE,num,fp);
			fread(dstOff,CONST_SECSIZE,num,fp); //可能返回0
			fclose(fp);
			return num;
	}else{
		return 0;
	}

}
int EnvInterface64Impl::printf_simple(const char *fmt,int arg0,int arg1,int arg2)
{
//	va_list ap;
//	va_start(ap,fmt);
//	vfprintf(stdout,fmt,ap);
//	va_end(ap);
	::printf(fmt,arg0,arg1,arg2);

	return 0;

}
int EnvInterface64Impl::printf_sn(const char *s,size_t n)
{
#define MAX_SPACE 40
	if(n<MAX_SPACE)
	{
		char buffer[MAX_SPACE];
		for(size_t i=0;i<n;i++)
			buffer[i]=s[i];
		buffer[n]='\0';
		printf_simple(buffer);
	}else{
		printf_sn(s,MAX_SPACE-1);
		printf_sn(s+MAX_SPACE-1, n - MAX_SPACE + 1);
	}
	return 0;
#undef MAX_SPACE
}
u8_t* EnvInterface64Impl::malloc(size_t size)
{
	return (u8_t*)::malloc(size);
}


	void  EnvInterface64Impl::free(u8_t* p)
	{
		::free(p);
	}

	void EnvInterface64Impl::systemAbort(const char *msg,int code)
	{
		::printf("--ABORT-- code=%d,msg=\"%s\"",code,msg);
		flushOutputs();
		::abort();
	}
	void EnvInterface64Impl::flushOutputs()
	{
		::fflush(stdout);
	}


	char *EnvInterface64Impl::strcpy(char *s,const char *ct)
	{
		return ::strcpy(s, ct);
	}
	char *EnvInterface64Impl::strncpy(char *s,const char *ct,size_t n)
	{
		return ::strncpy(s,ct,n);
	}
	int EnvInterface64Impl::strncmp(const char *cs,const char *ct,size_t n)
	{
		return ::strncmp(cs,ct,n);
	}
	size_t EnvInterface64Impl::strlen(const char *cs)
	{
		return ::strlen(cs);
	}
	void *EnvInterface64Impl::memset(char *s,int c,size_t n)
	{
		return ::memset(s,c,n);
	}
	void *EnvInterface64Impl::memcpy(char *s,const char *ct,size_t n)
	{
		return ::memcpy(s,ct,n);
	}

	//================ class StdEnv64Impl
	std::vector<std::string> StdEnv64Impl::regexSplit(const std::regex& re, const std::string& s) {
		const std::string::const_iterator begin = s.cbegin();
		auto itBeg = begin;
		std::vector<std::string> res;
		std::smatch allmatch;

		while (itBeg != s.end() && std::regex_search(itBeg, s.cend(), allmatch, re)) {
			// pos,len 每次迭代时，当还有时,应将beg~pos之间保留，然后beg变换到length之后
			res.push_back(s.substr(itBeg - begin, allmatch.position()));
			auto len = allmatch.length() + allmatch.position();
			if (s.cend() - itBeg <= len) //超出范围
				break;
			itBeg += len;
		}
		if(itBeg!=s.end())
		{
			res.push_back(s.substr(itBeg - begin));
			itBeg = s.end();
		}

		return std::move(res);
	}

	std::vector<std::string> StdEnv64Impl::spaceSplit(const std::string& s) //static函数是不可继承的
		{
			return std::move(regexSplit(std::regex("\\s+"), s));
		}
	std::vector<std::string> StdEnv64Impl::pathSplit(const std::string& s) //static函数是不可继承的
		{
			return std::move(regexSplit(std::regex("/+"), s));
		}
	std::vector<std::string> StdEnv64Impl::cmdSplit(const std::string& s)
		{
			return std::move(regexSplit(std::regex(";+"), s));
		}
	int		StdEnv64Impl::atoi(const std::string & s)
	{
			return ::atoi(s.c_str());
	}



#endif