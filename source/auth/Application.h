#ifndef _APPLICATION_H_
#define _APPLICATION_H_
#include <string>
//�ļ�˵��
//Ӧ�ó�����
//��װ��������-��ʼ��-����-�����Ĺ���


class Appliction
{
public:
	Appliction();
	~Appliction();
	static Appliction & getRef();
	static Appliction * getPtr(){ return &getRef(); }

	bool Init(std::string filename, unsigned int index);
	void RunPump();
	void Stop();
protected:
	void _Stop();

};




































#endif
