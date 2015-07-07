#pragma warning(disable:4786)

#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <direct.h>

using namespace std;

#define VM_NUM 5 //Ȩֵ����
#define BODY_NUM 100 //ÿ�����������
#define PARENT_NUM sqrt(BODY_NUM) //ÿ�������и��׵�����
#define CHILD_NUM (BODY_NUM - PARENT_NUM) //ÿ�������к��ӵ�����
#define EXPAND 2000 //������
#define RANGE_RATIO 0.8 //��Χ����

#define MAX_GEN_NUM 1000 //�����Ŵ�����

//�����û��ѷ��ʵ��ܻ�ӭ��Ʒ��ϵ����Ŀǰû�����⺬�壬���ò���
#define ALPHA 0.5
#define BETA  0.5

#define PM 0.2 //���������
#define PC 0.8  //Ⱦɫ�彻����

#define NOT_VAR_GENE_NUM 0 //���������Ļ���������Ҫ�ǿ��Ǹ�λ���������������ܲ�ĸ���

enum
{
	CLICK = 0, //���	
	BUY = 1, //����
	COLLECT = 2, //�ղ�
	SHOP_CART = 3, //���ﳵ	
	WEL_BRAND = 4 //�ܻ�ӭƷ��
};

//���ڽṹ�������·ݺ���������
typedef struct _visit_date
{
	BYTE mon;
	BYTE day;

}VISIT_DATE, *PVISIT_DATE;

//��è����־�ṹ
typedef struct _tm_log
{
	ULONG user_id; //�û�ID
	ULONG brand_id; //Ʒ��ID
	BYTE type; //�û���Ϊ0�������1������2���ղأ�3�����ﳵ
	VISIT_DATE visit_date; //�û����ʵ�����

}TM_LOG, *PTM_LOG;

//�û�����־�ṹ
typedef struct _user_log
{	
	ULONG brand_id; //Ʒ��ID
	BYTE type; //�û���Ϊ0�������1������2���ղأ�3�����ﳵ
	VISIT_DATE visit_date; //�û����ʵ�����

}USER_LOG, *PUSER_LOG;

typedef vector<USER_LOG> USER_LOGS;

//Ʒ����־�ṹ
typedef struct _brand_log
{
	ULONG user_id; //�û�id;
	BYTE type; //�û���Ϊ0�������1������2���ղأ�3�����ﳵ
	VISIT_DATE visit_date; //�û����ʵ�����

}BRAND_LOG, *PBRAND_LOG;

//Ʒ�Ƶļ�ֵ�ṹ
typedef struct _brand_value
{
	ULONG id; //Ʒ�Ƶ�id
	LONG64 value; //Ʒ�Ƶļ�ֵ
	bool visit; //��Ʒ���Ƿ��û����ʹ�

}BRAND_VALUE, *PBRAND_VALUE;

typedef struct _id_f1
{
	ULONG body_id; // ����id
	float p_val; //׼ȷ��
	float r_val; //�ٻ���
	float f1_val; //�����F1ֵ

}ID_F1, *PID_F1;

typedef struct _user_value
{
	ULONG user_id; //�û�id
	ULONG val; //�û���ֵ

}USER_VALUE, *PUSER_VALUE;

//���Ʒ�Ƶļ�ֵ�����Ĳ�ͬ���ǣ�������ͳ�������û��ģ����������ĳ���û���
typedef struct _brand_value_ex
{
	ULONG id; //Ʒ�Ƶ�id
	ULONG value; //Ʒ�Ƶļ�ֵ

}BRAND_VALUE_EX, *PBRAND_VALUE_EX;

typedef struct _user_buy_rat
{
	ULONG user_id;
	float ratio;

}USER_BUY_RAT, *PUSER_BUY_RAT;

typedef vector<ULONG> ULONGS;
typedef vector<BRAND_VALUE_EX> BRANDS_VALUE_EX;
typedef vector<BRAND_LOG> BRAND_LOGS;
typedef map<ULONG, BRANDS_VALUE_EX> USERS_BRANDS;

vector<USER_BUY_RAT> users_buy_rat; //�û�������

const VISIT_DATE divi_date = {7, 15}; //7��15��Ϊ�������ڵķֽ��ߣ��ڴ�����ǰ�����������ڣ�ΪԤ��ģ��ڴ����ں�Ϊ��ʵ��

map<ULONG, USER_LOGS> users_logs; //�����û�����־���ݣ���һ��Ϊ�û�ID,�ڶ���Ϊ�û�����Ϊ��־
map<ULONG, USER_LOGS> users_logs_pre; //Ϊ���ڷֽ���֮ǰ�������û���־
map<ULONG, USER_LOGS> users_logs_rea; //Ϊ���ڷֽ���֮��������û���־

map<ULONG, BRAND_LOGS> brands_logs; //����Ʒ�Ƶ���־���ݣ���һ��ΪƷ��ID,�ڶ���ΪƷ�Ƶ���־
map<ULONG, BRAND_LOGS> brands_logs_pre; //Ϊ���ڷֽ���֮ǰ������Ʒ����־
map<ULONG, BRAND_LOGS> brands_logs_rea; //Ϊ���ڷֽ���֮�������Ʒ����־

map<ULONG, ULONGS> new_bodys; //��һ������

ULONG Vm[VM_NUM] = {0, 2000, 3408, 3875, 4506}; //����Ϊ������ܻ�ӭƷ�ơ��ղء����ﳵ�������Ȩֵ
const BYTE type_Vm[VM_NUM] = {0, 4, 2, 3, 1}; //�����������͵�Vm��ֵ��ӳ��,0�������1������2���ղأ�3�����ﳵ
BYTE gene_num[VM_NUM-2] = {0}; //������Ⱦɫ����Ҫ����Ļ���������λ��

void classify_user_data(char *filename); //��t_alibaba_data.csv�ļ��е����ݽ��з���洢
void create_first_generation(ULONG body_num = BODY_NUM); //����Vmֵ���ɵ�һ������
void get_best_Vm(); //�����õ�Ȩֵ
vector<BRAND_VALUE> get_top_brands_value(const ULONG *Vm_body); //��ȡ��ֵ��ǰ��Ʒ���б�
bool get_Vm_body(ULONG gen_num, ULONG body_id, ULONG *Vm_body); //��ȡָ������ָ�������Vmֵ
ID_F1 get_f1(ULONG gen_num, ULONG body_id); //��ȡָ�������F1ֵ
vector<USER_VALUE> get_top_users_value(const ULONG *Vm_body, const vector<BRAND_VALUE> *pbrands_value); //��ȡ��ֵ��ǰ���û��б�
USERS_BRANDS get_top_users_brands(const ULONG *Vm_body, const vector<USER_VALUE> *users_value, const vector<BRAND_VALUE> *pbrands_value); //��ȡ�ļ�ֵ��ǰ���û����ܻ����Ʒ��
float get_precision(const USERS_BRANDS *users_brands); //��ȡ׼ȷ��
float get_recall(const USERS_BRANDS *users_brands); //��ȡ�ٻ���
void create_next_generation(ULONG gen_num, vector<ID_F1> *ids_f1); //������һ������
map<ULONG, ULONGS> get_top_Vms(ULONG gen_num, vector<ID_F1> *ids_f1); //��ȡ����ǰ���Vm����Ȼ����Vm�ĺ���Ϊ��Χֵ
map<ULONG, ULONGS> get_var_champ(const map<ULONG, ULONGS> *top_Vms); //������ǰ��ĸ�����л������
void cross_champ(const map<ULONG, ULONGS> *var_top_Vms, ULONG gen_num); //���и����Ⱦɫ�彻�沢������һ������
void write_gen_top_body_f1(ULONG gen_num, ID_F1 id_f1); //��ÿһ����F1ֵ��ߵĸ����¼����
void get_best_Vm_ex();
void get_users_buy_rat();
int get_pre_brand_num(ULONG user_id, ULONG sum_brand_num);
vector<USER_VALUE> get_top_users_brands_ex(const ULONG *Vm_body, const vector<BRAND_VALUE> *pbrands_value); //��ȡ�ļ�ֵ��ǰ���û����ܻ����Ʒ��

bool compare_user_visit_date(const USER_LOG &user_log1, const USER_LOG &user_log2) //�������ڣ���С��������Ļص������������ṹΪ�û���־
{
	if(user_log1.visit_date.mon< user_log2.visit_date.mon)
		return true;
	else if(user_log1.visit_date.mon> user_log2.visit_date.mon)
		return false;
	else if(user_log1.visit_date.day< user_log2.visit_date.day)
		return true;
	else
		return false;
}

bool compare_brand_visit_date(const BRAND_LOG &brand_log1, const BRAND_LOG &brand_log2) //�������ڣ���С��������Ļص������������ṹΪƷ����־
{
	if(brand_log1.visit_date.mon< brand_log2.visit_date.mon)
		return true;
	else if(brand_log1.visit_date.mon> brand_log2.visit_date.mon)
		return false;
	else if(brand_log1.visit_date.day< brand_log2.visit_date.day)
		return true;
	else
		return false;
}

bool compare_brand_id(const TM_LOG &tm_log1, const TM_LOG &tm_log2) //����Ʒ��id����С��������ĺ���
{
	return tm_log1.brand_id<tm_log2.brand_id;
}

bool compare_brand_id_ex(const USER_LOG &tm_log1, const USER_LOG &tm_log2) //����Ʒ��id����С��������ĺ���,�������û���־
{
	return tm_log1.brand_id<tm_log2.brand_id;
}

bool compare_brand_value(const BRAND_VALUE brand_value1, const BRAND_VALUE brand_value2) //����Ʒ�Ƽ�ֵ���Ӵ�С������
{
	return brand_value1.value>brand_value2.value;
}

bool compare_brand_value_ex(const BRAND_VALUE_EX brand_value_ex1, const BRAND_VALUE_EX brand_value_ex2) //����Ʒ�Ƽ�ֵ���Ӵ�С�����򣬲���Ϊ�����û�����־
{
	return brand_value_ex1.value >brand_value_ex2.value;
}

bool compare_user_value(const USER_VALUE user_value1, const USER_VALUE user_value2) //�����û���ֵ���Ӵ�С������
{
	return user_value1.val>user_value2.val;
}

bool compare_f1(const ID_F1 &id_f11, const ID_F1 &id_f12) //�Ѵ���ͬһ�������и��尴��F1ֵ�Ӵ�С����
{
	return id_f11.f1_val>id_f12.f1_val;
}

bool find_wel_brands(vector<BRAND_VALUE>* brands_value, ULONG brand_id)
{
	for(vector<BRAND_VALUE>::iterator iter = brands_value->begin(); iter!=brands_value->end(); iter++)
	{
		if(iter->id == brand_id)
		{
			iter->visit = true;
			return true;
		}			
	}
	return false;
}

int compare_visit_date_ext(const VISIT_DATE &visit_date1, const VISIT_DATE &visit_date2)
{
	if(visit_date1.mon> visit_date2.mon)
		return 1;
	else if(visit_date1.mon< visit_date2.mon)
		return -1;
	else if(visit_date1.day> visit_date2.day)
		return 1;
	else if(visit_date1.day< visit_date2.day)
		return -1;
	else
		return 0;
}

bool find_brand(const USER_LOGS *user_logs, ULONG brand_id, BYTE type)
{
	for(USER_LOGS::const_iterator iter = user_logs->begin(); iter != user_logs->end(); iter++)
	{
		if((iter->brand_id == brand_id) && (iter->type == type))
			return true;
	}
	return false;
}

bool find_brand_ex(const BRANDS_VALUE_EX *brands_value_ex, ULONG brand_id)
{
	for(BRANDS_VALUE_EX::const_iterator iter = brands_value_ex->begin(); iter != brands_value_ex->end(); iter++)
	{
		if(iter->id == brand_id)
			return true;
	}
	return false;
}

bool find_brand_type(ULONG brand_id, BYTE type)
{
	for(map<ULONG, BRAND_LOGS>::iterator iter1 = brands_logs_rea.begin(); iter1 != brands_logs_rea.end(); iter1++)
	{
		if(iter1->first != brand_id)continue;

		for(BRAND_LOGS:: iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			if(iter2->type == BUY)
			{
				return true;
			}
		}
	}
	return false;
}

bool find_user_brand(ULONG user_id, ULONG brand_id, BYTE type)
{
	for(map<ULONG, USER_LOGS>::iterator iter1 = users_logs_rea.begin(); iter1 != users_logs_rea.end(); iter1++)
	{
		if(iter1->first != user_id)continue;
		for(USER_LOGS::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			if(iter2->brand_id == brand_id && type == iter2->type)
				return true;
		}
	}
	return false;
}

ULONG get_bit_num(const ULONG &num)
{
	int i = 31;
	while(!(num & (1<<i--)));
	return i+2;
}

int main(int argc, char* argv[])
{
	char *filename = "t_alibaba_data.csv";
	classify_user_data(filename); //�Ȱ��������û�Ϊ��λ�����շ������ڽ�������	
	get_users_buy_rat();
	get_best_Vm(); //�����õ�Ȩֵ	
	//get_best_Vm_ex();
	return 0;
}

void classify_user_data(char *filename)
{	
	USER_LOG user_log;
	memset(&user_log, 0, sizeof(USER_LOG));
	ULONG user_id = 0, brand_id = 0, type = 0, mon = 0, day = 0;
	ULONG temp_user_id = 0;	
	USER_LOGS user_logs;

	FILE* file = fopen(filename, "r");	

	if(file &&!feof(file))
	{
		if(fscanf(file, "%lu,%lu,%u,%u��%u��", &user_id, &brand_id, &type, &mon, &day)!=5)
		{
			fclose(file);
			return;
		}
		user_log.brand_id = brand_id, user_log.type = type, user_log.visit_date.mon = mon, user_log.visit_date.day = day;
		user_logs.push_back(user_log); //�洢��һ���û��ĵ�һ����Ϊ��־

		temp_user_id = user_id;			
	}
	while(!feof(file))
	{		
		if(fscanf(file, "%lu,%lu,%u,%u��%u��", &user_id, &brand_id, &type, &mon, &day)!=5)
		{			
			break;
		}

		if(user_id != temp_user_id)
		{
			users_logs[temp_user_id] = user_logs; //�洢�м��û�����Ϊ��־
			user_logs.clear();
			temp_user_id = user_id;
		}

		user_log.brand_id = brand_id, user_log.type = type, user_log.visit_date.mon = mon, user_log.visit_date.day = day;
		user_logs.push_back(user_log);		
	}	
	users_logs[temp_user_id] = user_logs; //�洢���һ���û�����Ϊ��־

	fclose(file);	

	for(map<ULONG, USER_LOGS>::iterator iter1 = users_logs.begin(); iter1!=users_logs.end(); iter1++)
	{
		sort(iter1->second.begin(), iter1->second.end(), compare_user_visit_date); //�������ڴ�С�����������

		/*char filename[MAX_PATH] = {0};
		char user_id[10] = {0};		
		_itoa(iter1->first, user_id, 10);
		strcpy(filename, "users_logs\\sort_date\\"), strcat(filename, user_id), strcat(filename, ".txt");
		FILE * file = fopen(filename, "w");
		for(USER_LOGS::iterator iter2 = iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
		{
			fprintf(file, "%d,%d,%d,%d��%d��\n", iter1->first, iter2->brand_id, iter2->type, iter2->visit_date.mon, iter2->visit_date.day);
		}
		fclose(file);*/
	}

	for(iter1 = users_logs.begin(); iter1!=users_logs.end(); iter1++)
	{
		USER_LOGS user_logs_pre, user_logs_rea;
		for(USER_LOGS::iterator iter2 = iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
		{
			if(compare_visit_date_ext(iter2->visit_date, divi_date)<=0) //���û���־��divi_dateΪ���޷ֳ�������
			{
				user_logs_pre.push_back(*iter2);
			}
			else
			{
				user_logs_rea.push_back(*iter2);
			}
		}
		if(user_logs_pre.size()>0)
			users_logs_pre[iter1->first] = user_logs_pre;
		if(user_logs_rea.size()>0)
			users_logs_rea[iter1->first] = user_logs_rea;
	}

	/***********�����Ƕ�Ʒ��id���з���*******************************/
	vector<TM_LOG> tm_logs;
	for(iter1 = users_logs.begin(); iter1!=users_logs.end(); iter1++)
	{		
		for(USER_LOGS::iterator iter2 = iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
		{
			TM_LOG temp_tm_log;
			temp_tm_log.user_id = iter1->first;
			temp_tm_log.brand_id = iter2->brand_id;
			temp_tm_log.type = iter2->type;
			temp_tm_log.visit_date = iter2->visit_date;
			tm_logs.push_back(temp_tm_log);
		}
	}
	sort(tm_logs.begin(), tm_logs.end(), compare_brand_id); //����Ʒ��id��С�����������
		
	BRAND_LOG brand_log;
	memset(&brand_log, 0, sizeof(BRAND_LOG));	
	ULONG temp_brand_id = 0;	
	BRAND_LOGS brand_logs;	

	if(tm_logs.size()>0)
	{		
		brand_log.user_id = tm_logs[0].user_id, brand_log.type = tm_logs[0].type, brand_log.visit_date.mon = tm_logs[0].visit_date.mon, brand_log.visit_date.day = tm_logs[0].visit_date.day;
		brand_logs.push_back(brand_log); //�洢��һ��Ʒ�Ƶĵ�һ���û���־

		temp_brand_id = tm_logs[0].brand_id;			
	}
	for(vector<TM_LOG>::iterator iter = tm_logs.begin()+1; iter!=tm_logs.end(); iter++)
	{		
		if(iter->brand_id != temp_brand_id)
		{
			brands_logs[temp_brand_id] = brand_logs; //�洢�м�Ʒ�Ƶ���־
			brand_logs.clear();
			temp_brand_id = iter->brand_id;
		}

		brand_log.user_id = iter->user_id, brand_log.type = iter->type, brand_log.visit_date.mon = iter->visit_date.mon, brand_log.visit_date.day = iter->visit_date.day;
		brand_logs.push_back(brand_log);		
	}	
	brands_logs[temp_brand_id] = brand_logs; //�洢���һ��Ʒ�Ƶ���־	

	for(map<ULONG, BRAND_LOGS>::iterator iter3 = brands_logs.begin(); iter3!=brands_logs.end(); iter3++)
	{
		sort(iter3->second.begin(), iter3->second.end(), compare_brand_visit_date); //�������ڴ�С�����������

		/*char filename[MAX_PATH] = {0};
		char brand_id[10] = {0};		
		_itoa(iter3->first, brand_id, 10);
		strcpy(filename, "brands_logs\\sort_date\\"), strcat(filename, brand_id), strcat(filename, ".txt");
		FILE * file = fopen(filename, "w");
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			fprintf(file, "%d,%d,%d,%d��%d��\n", iter4->user_id, iter3->first, iter4->type, iter4->visit_date.mon, iter4->visit_date.day);
		}
		fclose(file);*/
	}

	for(iter3 = brands_logs.begin(); iter3!=brands_logs.end(); iter3++)
	{
		BRAND_LOGS brand_logs_pre, brand_logs_rea;
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			if(compare_visit_date_ext(iter4->visit_date, divi_date)<=0) //��Ʒ����־��divi_dateΪ���޷ֳ�������
			{
				brand_logs_pre.push_back(*iter4);
			}
			else
			{
				brand_logs_rea.push_back(*iter4);
			}
		}
		if(brand_logs_pre.size()>0)
			brands_logs_pre[iter3->first] = brand_logs_pre;
		if(brand_logs_rea.size()>0)
			brands_logs_rea[iter3->first] = brand_logs_rea;
	}

	/*file = fopen("test4.txt", "w"); //��Ҫ����ȡ���ڴ������ݸ�ʽ�Ƿ���ȷ
	for(iter3 = brands_logs_pre.begin(); iter3!=brands_logs_pre.end(); iter3++)
	{
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			fprintf(file, "%d,%d,%d,%d��%d��\n", iter4->user_id, iter3->first, iter4->type, iter4->visit_date.mon, iter4->visit_date.day);
		}
	}
	fclose(file);

	file = fopen("test5.txt", "w"); //��Ҫ����ȡ���ڴ������ݸ�ʽ�Ƿ���ȷ
	for(iter3 = brands_logs_rea.begin(); iter3!=brands_logs_rea.end(); iter3++)
	{
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			fprintf(file, "%d,%d,%d,%d��%d��\n", iter4->user_id, iter3->first, iter4->type, iter4->visit_date.mon, iter4->visit_date.day);
		}
	}
	fclose(file);*/

	return;
}

void create_first_generation(ULONG body_num)
{		
	for(int i =0; i< VM_NUM; i++)
	{
		Vm[i] *= EXPAND; //������Ȩֵ		
	}

	ULONG Vm_range[VM_NUM-2] = {0};
	for(i = 1; i<VM_NUM-1; i++)
	{
		Vm_range[i-1] = (Vm[i+1] - Vm[i-1])/2;		
		Vm_range[i-1] *= RANGE_RATIO; //ȷ����Χ��С
		gene_num[i-1] = get_bit_num(Vm_range[i-1]) - NOT_VAR_GENE_NUM;
	}	
	
	for(i = VM_NUM-2; i>0; i--)
	{		
		Vm[i] = (Vm[i] + Vm[i-1])/2; //���µ���Vmֵ���Ա���Ϸ�Χֵ��ֱ�ӵõ�Ȩֵ		
	}

	FILE *file = NULL;
	char filename[MAX_PATH] = {0};
	for(i = 0; i< BODY_NUM; i++)
	{
		char num[10] = {0};
		strcpy(filename, "GA_record\\generation_1\\"), _mkdir(filename), _itoa(i+1, num, 10), strcat(filename, num), strcat(filename, ".txt");
		file = fopen(filename, "w");
		for(int j =0; j< VM_NUM-2; j++)
		{
			fprintf(file, "%d\n", rand()%Vm_range[j]); //�Ѹ�������ֵд���ļ�
		}
		fclose(file);
	}
	return;
}

void get_best_Vm()
{
	create_first_generation(); //���ɵ�һ������
	for(int gen_num = 1; gen_num <= MAX_GEN_NUM; gen_num++)
	{
		printf("gen_num:\t%d\n", gen_num);
		vector<ID_F1> ids_f1;		
		for(int body_id = 1; body_id <= BODY_NUM; body_id++)
		{
			ID_F1 id_f1;			
			id_f1 = get_f1(gen_num, body_id);			

			ids_f1.push_back(id_f1);			
		}
		sort(ids_f1.begin(), ids_f1.end(), compare_f1);
		ids_f1.resize(sqrt(BODY_NUM)); //ȡǰsqrt(BODY_NUM)��Ŀ��������������������������岢���ϱ���պ�ΪBODY_NUM��

		create_next_generation(gen_num, &ids_f1); //������һ������

		write_gen_top_body_f1(gen_num, ids_f1[0]); //��ÿһ����F1ֵ��ߵĸ����¼����
	}	
	return;
}

vector<BRAND_VALUE> get_top_brands_value(const ULONG *Vm_body)
{
	vector<BRAND_VALUE> brands_value;
	for(map<ULONG, BRAND_LOGS>::iterator iter1 = brands_logs_pre.begin(); iter1!=brands_logs_pre.end(); iter1++)
	{
		BRAND_VALUE brand_value;
		memset(&brand_value, 0, sizeof(BRAND_VALUE));
		brand_value.id = iter1->first;
		brand_value.visit = false; //��ʼ��Ϊ����Ʒ��û�б����ʹ�
		for(BRAND_LOGS::iterator iter2 = iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
		{					
			brand_value.value += Vm_body[type_Vm[iter2->type]]; //Ʒ�Ƶļ�ֵ���ȽϺ�����㷨���ǰ��������Ʒ���йص���ΪȨֵ���
		}
		brands_value.push_back(brand_value);
	}
	sort(brands_value.begin(), brands_value.end(), compare_brand_value); //����Ʒ�Ƽ�ֵ�Ӵ�С����

	int size = brands_value.size(); //����Ʒ�Ƹ���
	int top = sqrt(size); //Ҫѡȡ���ܻ�ӭƷ����
	brands_value.resize(top); //ȡǰtop��
	return brands_value;
}

bool get_Vm_body(ULONG gen_num, ULONG body_id, ULONG *Vm_body)
{
	char filename[MAX_PATH] = {0};
	char str_gen_num[10] = {0}, str_body_id[10] = {0};
	strcpy(filename, "GA_record\\"), strcat(filename, "generation_"), _itoa(gen_num, str_gen_num, 10), strcat(filename, str_gen_num), 
	strcat(filename, "\\"), _itoa(body_id, str_body_id, 10), strcat(filename, str_body_id), strcat(filename, ".txt");

	FILE* file = fopen(filename, "r");
	if(!file)
	{
		printf("\"%s\" do not exist!\n", filename);
		return false;
	}

	Vm_body[0] = Vm[0]; //��һ��Vmֵ����
	for(int i = 1; i< VM_NUM-1; i++)
	{
		ULONG range = 0;
		if(fscanf(file, "%d\n", &range) != 1)//��ȡ�Ķ���Vm�ķ�Χֵ
		{
			printf("\"%s\" content is not correct!\n", filename);
			return false;
		}
		Vm_body[i] = Vm[i] + range; //����Vmֵ���Ϸ�Χֵ����ĳ�����������Vmֵ
	}
	Vm_body[VM_NUM-1] = Vm[VM_NUM-1]; //���һ��VmֵҲ����

	fclose(file);
	return true;
}

ID_F1 get_f1(ULONG gen_num, ULONG body_id)
{
	ID_F1 id_f1;

	float f1 = 0.0; // �ۺ�ֵ
	float p = 0.0; //׼ȷ��
	float r = 0.0; //�ٻ���

	ULONG Vm_body[VM_NUM] = {0};
	vector<BRAND_VALUE> brands_value; //��ֵ��ǰ��Ʒ��
	vector<USER_VALUE> users_value; //��ֵ��ǰ���û�
	USERS_BRANDS users_brands; //���ջ�ȡ�ļ�ֵ��ǰ���û����ܻ����Ʒ��

	if(!get_Vm_body(gen_num, body_id, Vm_body)) //��ȡָ������ָ�������Vmֵ
	{
		printf("get_Vm_body failed!\n");		
		exit(0);
	}
	brands_value = get_top_brands_value(Vm_body); //��ȡ��ֵ��ǰ��Ʒ���б�	
	//users_value = get_top_users_value(Vm_body, &brands_value); //��ȡ��ֵ��ǰ���û��б�
	//users_brands = get_top_users_brands(Vm_body, &users_value, &brands_value); //��ȡ�ļ�ֵ��ǰ���û����ܻ����Ʒ��
	users_value = get_top_users_brands_ex(Vm_body, &brands_value); //��ȡ�ļ�ֵ��ǰ���û����ܻ����Ʒ��
	users_brands = get_top_users_brands(Vm_body, &users_value, &brands_value); //��ȡ�ļ�ֵ��ǰ���û����ܻ����Ʒ��

	p = get_precision(&users_brands);
	r = get_recall(&users_brands);
	f1 = 2*p*r/(p+r);

	id_f1.body_id = body_id;
	id_f1.p_val = p;
	id_f1.r_val = r;
	id_f1.f1_val = f1;

	printf("%2.2f\t", p*100);
	printf("%2.2f\t", r*100);
	printf("%2.2f\n", f1*100);

	return id_f1;
}

vector<USER_VALUE> get_top_users_value(const ULONG *Vm_body, const vector<BRAND_VALUE> *pbrands_value)
{
	vector<USER_VALUE> users_value;

	for(map<ULONG, USER_LOGS>::iterator iter1 = users_logs_pre.begin(); iter1 != users_logs_pre.end(); iter1++)
	{
		vector<BRAND_VALUE> brands_value = *pbrands_value;
		ULONG sum_val = 0;
		USER_VALUE user_value;
		user_value.user_id = iter1->first;
		for(USER_LOGS::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{		
			if(find_wel_brands(&brands_value, iter2->brand_id)) //����û����ʵ����ܻ�ӭ��Ʒ�ƣ�Ҫ��������Ĺ�ʽ
			{
				sum_val += pow(Vm_body[type_Vm[iter2->type]], 2) / Vm_body[type_Vm[WEL_BRAND]]; //��������Ŀ����Ҫ�����û��Ƿ��ϲ���ܻ�ӭ��Ʒ��
			}
			else
			{
				sum_val += Vm_body[type_Vm[iter2->type]]; //�Դ��û������в��������ۼӣ���ʱ�������ܻ�ӭƷ��
			}			
		}
		user_value.val = sum_val;
		users_value.push_back(user_value);
	}
	sort(users_value.begin(), users_value.end(), compare_user_value); //����Ʒ�Ƽ�ֵ�Ӵ�С����

	int size = users_value.size(); //�����û�����
	int top = sqrt(size); //Ҫѡȡ���м�ֵ���û���
	users_value.resize(187); //ȡǰtop��
	return users_value;
}

USERS_BRANDS get_top_users_brands(const ULONG *Vm_body, const vector<USER_VALUE> *users_value, const vector<BRAND_VALUE> *pbrands_value)
{
	USERS_BRANDS users_brands;
	BRANDS_VALUE_EX brands_value_ex;
	BRAND_VALUE_EX brand_value_ex;
	ULONG temp_brand_id = 0;
	for(vector<USER_VALUE>::const_iterator iter1 = users_value->begin(); iter1 != users_value->end(); iter1 ++)
	{		
		bool found = false;
		vector<BRAND_VALUE> brands_value = *pbrands_value; //�ܻ�ӭ��Ʒ���б�

		USER_LOGS user_logs = users_logs_pre[iter1->user_id];
		sort(user_logs.begin(), user_logs.end(), compare_brand_id_ex); //��Ʒ�ƴ�С��������Ŀ���ǰ���ͬƷ�Ƶ��û���־����һ��
		
		memset(&brand_value_ex, 0 ,sizeof(BRAND_VALUE_EX));

		brand_value_ex.id = user_logs[0].brand_id;
		if(find_wel_brands(&brands_value, brand_value_ex.id))
		{
			brand_value_ex.value = pow(Vm_body[type_Vm[user_logs[0].type]], 2) / Vm_body[type_Vm[WEL_BRAND]];
			found = true;
		}		
		else
		{
			brand_value_ex.value = Vm_body[type_Vm[user_logs[0].type]];
			found = false;
		}

		//brand_value_ex.value = Vm_body[type_Vm[user_logs[0].type]];
		temp_brand_id = user_logs[0].brand_id;

		for(USER_LOGS::iterator iter2 = user_logs.begin()+1; iter2 != user_logs.end(); iter2++)
		{
			if(iter2->brand_id != temp_brand_id) //ÿ���������ϴβ���ͬ��Ʒ��ID�����¼���
			{
				brands_value_ex.push_back(brand_value_ex); //�洢���û��ĸ���Ʒ�Ƽ�ֵ�б�

				brand_value_ex.id = iter2->brand_id;
				if(find_wel_brands(&brands_value, brand_value_ex.id))
				{
					brand_value_ex.value = pow(Vm_body[type_Vm[iter2->type]], 2) / Vm_body[type_Vm[WEL_BRAND]];
					found = true;
				}		
				else
				{
					brand_value_ex.value = Vm_body[type_Vm[iter2->type]];
					found = false;
				}			

				//brand_value_ex.value = Vm_body[type_Vm[iter2->type]];
				temp_brand_id = iter2->brand_id;
				continue;
			}
			if(found)
			{
				brand_value_ex.value += pow(Vm_body[type_Vm[iter2->type]], 2) / Vm_body[type_Vm[WEL_BRAND]];
			}
			else
			{
				brand_value_ex.value += Vm_body[type_Vm[iter2->type]];
			}
			brand_value_ex.value += Vm_body[type_Vm[iter2->type]];
		}

		/*for(vector<BRAND_VALUE>::iterator iter = brands_value.begin(); iter != brands_value.end(); iter ++) //���ܻ�ӭ��Ʒ�Ƶ��Ƽ�
		{
			if(!iter->visit) //����û�û�з��ʴ�Ʒ�ƣ��͸��û��Ƽ���Ʒ�ƣ���Ȼ��Ʒ���Ƿ����Ƽ���ȥ�����ÿ�����
			{
				brand_value_ex.id = iter->id;
				brand_value_ex.value = Vm_body[type_Vm[WEL_BRAND]];
				brands_value_ex.push_back(brand_value_ex);
			}			
		}*/
		sort(brands_value_ex.begin(), brands_value_ex.end(), compare_brand_value_ex); //�Ȱ���Ʒ�Ƽ�ֵ�Ӵ�С����
		int size = brands_value_ex.size();
		int top = sqrt(size);
		top = get_pre_brand_num(iter1->user_id, size);
		brands_value_ex.resize(top); //ȡǰtop��
		users_brands[iter1->user_id] = brands_value_ex;
	}
	return users_brands;
}

float get_precision(const USERS_BRANDS *users_brands)
{
	float p = 0.0;
	ULONG sum_p_brand = 0, sum_hit_brand = 0;
	for(USERS_BRANDS::const_iterator iter1 = users_brands->begin(); iter1 != users_brands->end(); iter1++)
	{
		for(BRANDS_VALUE_EX::const_iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			sum_p_brand++;
			if(find_brand(&users_logs_rea[iter1->first], iter2->id, BUY))
			{
				sum_hit_brand++;
			}
		}
	}
	p = (float)sum_hit_brand/sum_p_brand;	
	return p;
}

float get_recall(const USERS_BRANDS *users_brands)
{
	float r = 0.0;
	ULONG sum_b_brand = 0, sum_hit_brand = 0;
	USERS_BRANDS users_brands_temp = *users_brands;
	for(map<ULONG, USER_LOGS>::iterator iter1 = users_logs_rea.begin(); iter1 != users_logs_rea.end(); iter1++)
	{
		for(USER_LOGS:: iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			if(iter2->type == BUY)
			{
				sum_b_brand++;
				if(users_brands_temp[iter1->first].size()>0 &&find_brand_ex(&users_brands_temp[iter1->first], iter2->brand_id))
				{
					sum_hit_brand++;
				}
			}
		}
	}
	r = (float)sum_hit_brand/sum_b_brand;	
	return r;
}

void create_next_generation(ULONG gen_num, vector<ID_F1> *ids_f1)
{
	map<ULONG, ULONGS> top_Vms = get_top_Vms(gen_num, ids_f1); //��ȡ����ǰ���Vm�ķ�Χֵ��������
	
	map<ULONG, ULONGS> var_top_Vms = get_var_champ(&top_Vms); //��ȡ�����ĸ���

	cross_champ(&var_top_Vms, gen_num); //���и����Ⱦɫ�彻�沢������һ������
	return;
}

map<ULONG, ULONGS> get_top_Vms(ULONG gen_num, vector<ID_F1> *ids_f1)
{	
	map<ULONG, ULONGS> top_Vms;
	for(vector<ID_F1>::iterator iter = ids_f1->begin(); iter != ids_f1->end(); iter++)
	{
		ULONGS Vm_body;

		char filename[MAX_PATH] = {0};
		char str_gen_num[10] = {0}, str_body_id[10] = {0};		

		strcpy(filename, "GA_record\\"), strcat(filename, "generation_"), _itoa(gen_num, str_gen_num, 10), strcat(filename, str_gen_num), 
		strcat(filename, "\\"), _itoa(iter->body_id, str_body_id, 10), strcat(filename, str_body_id), strcat(filename, ".txt");

		FILE* file = fopen(filename, "r");
		if(!file)
		{
			printf("\"%s\" do not exist!\n", filename);
			printf("press any key to quit...");
			getch();
			exit(0);
		}
				
		for(int i = 1; i< VM_NUM-1; i++)
		{
			ULONG range = 0;
			if(fscanf(file, "%d\n", &range) != 1)//��ȡ�Ķ���Vm�ķ�Χֵ
			{
				printf("\"%s\" content is not correct!\n", filename);
				printf("press any key to quit...");
				getch();
				exit(0);
			}
			Vm_body.push_back(range);
		}		
	
 		top_Vms[iter->body_id] = Vm_body;
		fclose(file);
	}

	int count = 0 ;
	new_bodys.clear();	
	for(map<ULONG, ULONGS>::iterator iter_t = top_Vms.begin(); iter_t != top_Vms.end(); iter_t++, count++)
	{
		new_bodys[count] = iter_t->second;  //������ǰ��ĸ����������һ�����¸�����
	}

	return top_Vms;
}

map<ULONG, ULONGS> get_var_champ(const map<ULONG, ULONGS> *top_Vms)
{
	map<ULONG, ULONGS> top_Vms_ret = *top_Vms;

	for(map<ULONG, ULONGS>::iterator iter1 = top_Vms_ret.begin(); iter1 != top_Vms_ret.end(); iter1++)
	{
		for(ULONGS::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{		
			int index = 0;
			for(int k=0;k<gene_num[index];k++)
			{
				if(rand()%RAND_MAX < PM*RAND_MAX)
					(*iter2) ^= (1<<k);
			}
			index++;
		}
	}
	return top_Vms_ret;
}

void cross_champ(const map<ULONG, ULONGS> *var_top_Vms, ULONG gen_num)
{	
	int top_size = var_top_Vms->size();
	int count = 0;
	int first = 0, second = 0;
			
	map<ULONG, ULONGS> var_top_bodys;
	for(map<ULONG, ULONGS>::const_iterator iter_t = var_top_Vms->begin(); iter_t != var_top_Vms->end(); iter_t++, count++)
	{
		var_top_bodys[count] = iter_t->second;
	}
	while(count < BODY_NUM)
	{
		first=rand()%top_size;
		second=rand()%top_size;
		if(first==second)continue;				

		ULONGS first_body = var_top_bodys[first];
		ULONGS second_body = var_top_bodys[second];
		for(int i = 0; i< VM_NUM-2; i++)	
		{
			if(rand()%RAND_MAX < PC*RAND_MAX)
			{
				int t = first_body[i];
				first_body[i] = second_body[i];
				second_body[i] = t;
			}
		}
		new_bodys[count] = first_body;
		new_bodys[count+1] = second_body;
		count+=2;		
	}

	FILE *file = NULL;
	char filename[MAX_PATH] = {0};
	char str_gen_num[10] = {0};
	char str_body_id[10] = {0};
	for(map<ULONG, ULONGS>::iterator iter1 = new_bodys.begin(); iter1 != new_bodys.end(); iter1++)
	{		
		strcpy(filename, "GA_record\\"), strcat(filename, "generation_"), _itoa(gen_num+1, str_gen_num, 10), strcat(filename, str_gen_num), _mkdir(filename), 
		strcat(filename, "\\"), _itoa(iter1->first+1, str_body_id, 10), strcat(filename, str_body_id), strcat(filename, ".txt");
		file = fopen(filename, "w");
		for(ULONGS::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			int val = (*iter2);
			fprintf(file, "%d\n", val); //�Ѹ�������ֵд���ļ�
		}
		fclose(file);
	}
	return;
}

void write_gen_top_body_f1(ULONG gen_num, ID_F1 id_f1)
{
	static bool first = true;
	FILE * file = NULL;
	if(first)
	{
		file = fopen("top_body_f1.txt", "w");
		first = false;
	}
	else
	{
		file = fopen("top_body_f1.txt", "a");
	}

	SYSTEMTIME st;
	::GetLocalTime(&st);	
	fprintf(file,"%5d\t%5d\t%2.2f\t%2.2f\t%2.2f\t", gen_num, id_f1.body_id, id_f1.p_val*100, id_f1.r_val*100, id_f1.f1_val*100);
	fprintf(file,"time: %2d : %2d : %2d\n", st.wHour, st.wMinute, st.wSecond);
	fclose(file);
}

void get_best_Vm_ex()
{
	int sum_buy_rea = 0; //���ĸ��¹�������
	int sum_buy_pre = 0; //ǰ�����¹�������
	int sum_col_pre = 0; //ǰ�������ղ�����
	int sum_car_pre = 0; //ǰ�����¹��ﳵ����

	int sum_buy_pre_rea = 0; //ǰ����������ͬƷ�ƹ������͵��ĸ�����ͬƷ�ƵĹ������Ľ���
	int sum_col_pre_rea = 0; //ǰ����������ͬƷ���ղ����͵��ĸ�����ͬƷ�ƵĹ������Ľ���
	int sum_car_pre_rea = 0; //ǰ����������ͬƷ�ƹ��ﳵ���͵��ĸ�����ͬƷ�ƵĹ������Ľ���
	for(map<ULONG, BRAND_LOGS>::iterator iter1 = brands_logs_rea.begin(); iter1 != brands_logs_rea.end(); iter1++)
	{
		for(BRAND_LOGS:: iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			if(iter2->type == BUY)
			{
				sum_buy_rea++;
			}
		}
	}

	int sum_buy_user = 0;
	for(map<ULONG, USER_LOGS>::iterator iter_u = users_logs.begin(); iter_u != users_logs.end(); iter_u++)
	{
		for(USER_LOGS:: iterator iter2 = iter_u->second.begin(); iter2 != iter_u->second.end(); iter2++)
		{
			if(iter2->type == BUY)
			{
				sum_buy_user++;
				break;
			}
		}
	}
	printf("sum_buy_user: %d\n", sum_buy_user/4);

	for(iter1 = brands_logs_pre.begin(); iter1 != brands_logs_pre.end(); iter1++)
	{
		for(BRAND_LOGS:: iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			if(iter2->type == BUY)
			{
				if(find_brand_type(iter1->first, BUY))
				{
					sum_buy_pre_rea++;
				}
				sum_buy_pre++;
			}
			if(iter2->type == COLLECT)
			{
				if(find_brand_type(iter1->first, BUY))
				{
					sum_col_pre_rea++;
				}
				sum_col_pre++;
			}
			if(iter2->type == SHOP_CART)
			{
				if(find_brand_type(iter1->first, BUY))
				{
					sum_car_pre_rea++;
				}
				sum_car_pre++;
			}
		}
	}
	
	float buy_rat = (float)sum_buy_pre_rea/sum_buy_pre;
	float col_rat = (float)sum_col_pre_rea/sum_col_pre;
	float car_rat = (float)sum_car_pre_rea/sum_car_pre;
	printf("%2.4f\n%2.4f\n%2.4f\n", buy_rat, col_rat, car_rat);
	return;
}

void get_users_buy_rat()
{
	for(map<ULONG, USER_LOGS>::iterator iter_u = users_logs.begin(); iter_u != users_logs.end(); iter_u++)
	{
		int sum_buy = 0, sum = 0;
		for(USER_LOGS:: iterator iter2 = iter_u->second.begin(); iter2 != iter_u->second.end(); iter2++)
		{
			if(iter2->type == BUY)
			{
				sum_buy++;				
			}
			sum++;
		}
		USER_BUY_RAT  user_buy_rat;
		user_buy_rat.user_id = iter_u->first;
		user_buy_rat.ratio = (float)sum_buy/sum;
		users_buy_rat.push_back(user_buy_rat);
	}
	return ;
}

int get_pre_brand_num(ULONG user_id, ULONG sum_brand_num)
{
	for(vector<USER_BUY_RAT>::iterator iter = users_buy_rat.begin(); iter != users_buy_rat.end(); iter++)
	{
		if(iter->user_id == user_id)
		{
			return sum_brand_num*iter->ratio;
		}
	}
	return 0;
}

vector<USER_VALUE> get_top_users_brands_ex(const ULONG *Vm_body, const vector<BRAND_VALUE> *pbrands_value)
{
	USERS_BRANDS users_brands;
	BRANDS_VALUE_EX brands_value_ex;
	BRAND_VALUE_EX brand_value_ex;
	ULONG temp_brand_id = 0;
	vector<BRAND_VALUE> brands_value;
	USER_LOGS user_logs;
	bool found;
	for(map<ULONG, USER_LOGS>::iterator iter1 = users_logs_pre.begin(); iter1 != users_logs_pre.end(); iter1++)
	{		
		found = false;
		brands_value = *pbrands_value; //�ܻ�ӭ��Ʒ���б�

		user_logs = iter1->second;
		sort(user_logs.begin(), user_logs.end(), compare_brand_id_ex); //��Ʒ�ƴ�С��������Ŀ���ǰ���ͬƷ�Ƶ��û���־����һ��
		
		memset(&brand_value_ex, 0 ,sizeof(BRAND_VALUE_EX));

		brand_value_ex.id = user_logs[0].brand_id;
		if(find_wel_brands(&brands_value, brand_value_ex.id))
		{
			brand_value_ex.value = pow(Vm_body[type_Vm[user_logs[0].type]], 2) / Vm_body[type_Vm[WEL_BRAND]];
			found = true;
		}		
		else
		{
			brand_value_ex.value = Vm_body[type_Vm[user_logs[0].type]];
			found = false;
		}
		
		temp_brand_id = user_logs[0].brand_id;

		for(USER_LOGS::iterator iter2 = user_logs.begin()+1; iter2 != user_logs.end(); iter2++)
		{			
			if(iter2->type<CLICK || iter2->type>SHOP_CART)
			{
				printf("%d\n", iter1->first);
			}
			if(iter2->brand_id != temp_brand_id) //ÿ���������ϴβ���ͬ��Ʒ��ID�����¼���
			{
				brands_value_ex.push_back(brand_value_ex); //�洢���û��ĸ���Ʒ�Ƽ�ֵ�б�

				brand_value_ex.id = iter2->brand_id;
				if(find_wel_brands(&brands_value, brand_value_ex.id))
				{
					brand_value_ex.value = pow(Vm_body[type_Vm[iter2->type]], 2) / Vm_body[type_Vm[WEL_BRAND]];
					found = true;
				}		
				else
				{
					brand_value_ex.value = Vm_body[type_Vm[iter2->type]];
					found = false;
				}			

				//brand_value_ex.value = Vm_body[type_Vm[iter2->type]];
				temp_brand_id = iter2->brand_id;
				continue;
			}
			if(found)
			{
				brand_value_ex.value += pow(Vm_body[type_Vm[iter2->type]], 2) / Vm_body[type_Vm[WEL_BRAND]];
			}
			else
			{
				brand_value_ex.value += Vm_body[type_Vm[iter2->type]];
			}
			brand_value_ex.value += Vm_body[type_Vm[iter2->type]];
		}

		sort(brands_value_ex.begin(), brands_value_ex.end(), compare_brand_value_ex); //�Ȱ���Ʒ�Ƽ�ֵ�Ӵ�С����
		int size = brands_value_ex.size();
		int top = sqrt(size);
		top = get_pre_brand_num(iter1->first, size);
		brands_value_ex.resize(top); //ȡǰtop��
		users_brands[iter1->first] = brands_value_ex;
	}

	vector<USER_VALUE> users_value;

	for(USERS_BRANDS::iterator iter3 = users_brands.begin(); iter3!= users_brands.end(); iter3++)
	{
		USER_VALUE user_value;
		user_value.user_id = iter3->first;
		user_value.val = 0;
		for(BRANDS_VALUE_EX::iterator iter4 = iter3->second.begin(); iter4 != iter3->second.end(); iter4++)
		{
			if(find_user_brand(iter3->first, iter4->id, BUY))
			{
				user_value.val++;
			}
		}
		users_value.push_back(user_value);
	}
	sort(users_value.begin(), users_value.end(), compare_user_value); //����Ʒ�Ƽ�ֵ�Ӵ�С����

	int size = users_value.size(); //�����û�����
	int top = sqrt(size); //Ҫѡȡ���м�ֵ���û���
	users_value.resize(187); //ȡǰtop��
	return users_value;
}