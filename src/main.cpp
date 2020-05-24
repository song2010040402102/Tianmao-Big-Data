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

#define VM_NUM 5 //权值数量
#define BODY_NUM 100 //每代个体的数量
#define PARENT_NUM sqrt(BODY_NUM) //每代个体中父亲的数量
#define CHILD_NUM (BODY_NUM - PARENT_NUM) //每代个体中孩子的数量
#define EXPAND 2000 //扩大倍数
#define RANGE_RATIO 0.8 //范围比例

#define MAX_GEN_NUM 1000 //最大的遗传代数

//对于用户已访问的受欢迎的品牌系数，目前没有特殊含义，还用不上
#define ALPHA 0.5
#define BETA  0.5

#define PM 0.2 //基因变异率
#define PC 0.8  //染色体交叉率

#define NOT_VAR_GENE_NUM 0 //不参与变异的基因数，主要是考虑高位基因若变异会产生很差的个体

enum
{
	CLICK = 0, //点击
	BUY = 1, //购买
	COLLECT = 2, //收藏
	SHOP_CART = 3, //购物车
	WEL_BRAND = 4 //受欢迎品牌
};

//日期结构，包括月份和天数即可
typedef struct _visit_date
{
	BYTE mon;
	BYTE day;

}VISIT_DATE, *PVISIT_DATE;

//天猫的日志结构
typedef struct _tm_log
{
	ULONG user_id; //用户ID
	ULONG brand_id; //品牌ID
	BYTE type; //用户行为0：点击，1：购买，2：收藏，3：购物车
	VISIT_DATE visit_date; //用户访问的日期

}TM_LOG, *PTM_LOG;

//用户的日志结构
typedef struct _user_log
{
	ULONG brand_id; //品牌ID
	BYTE type; //用户行为0：点击，1：购买，2：收藏，3：购物车
	VISIT_DATE visit_date; //用户访问的日期

}USER_LOG, *PUSER_LOG;

typedef vector<USER_LOG> USER_LOGS;

//品牌日志结构
typedef struct _brand_log
{
	ULONG user_id; //用户id;
	BYTE type; //用户行为0：点击，1：购买，2：收藏，3：购物车
	VISIT_DATE visit_date; //用户访问的日期

}BRAND_LOG, *PBRAND_LOG;

//品牌的价值结构
typedef struct _brand_value
{
	ULONG id; //品牌的id
	LONG64 value; //品牌的价值
	bool visit; //此品牌是否被用户访问过

}BRAND_VALUE, *PBRAND_VALUE;

typedef struct _id_f1
{
	ULONG body_id; // 个体id
	float p_val; //准确率
	float r_val; //召回率
	float f1_val; //个体的F1值

}ID_F1, *PID_F1;

typedef struct _user_value
{
	ULONG user_id; //用户id
	ULONG val; //用户价值

}USER_VALUE, *PUSER_VALUE;

//这个品牌的价值和上文不同的是：上文是统计所有用户的，这里是针对某个用户的
typedef struct _brand_value_ex
{
	ULONG id; //品牌的id
	ULONG value; //品牌的价值

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

vector<USER_BUY_RAT> users_buy_rat; //用户购买率

const VISIT_DATE divi_date = {7, 15}; //7月15日为访问日期的分界线，在此日期前（包括此日期）为预测的，在此日期后为真实的

map<ULONG, USER_LOGS> users_logs; //所有用户的日志数据，第一项为用户ID,第二项为用户的行为日志
map<ULONG, USER_LOGS> users_logs_pre; //为日期分界线之前的所有用户日志
map<ULONG, USER_LOGS> users_logs_rea; //为日期分界线之后的所有用户日志

map<ULONG, BRAND_LOGS> brands_logs; //所有品牌的日志数据，第一项为品牌ID,第二项为品牌的日志
map<ULONG, BRAND_LOGS> brands_logs_pre; //为日期分界线之前的所有品牌日志
map<ULONG, BRAND_LOGS> brands_logs_rea; //为日期分界线之后的所有品牌日志

map<ULONG, ULONGS> new_bodys; //新一代个体

ULONG Vm[VM_NUM] = {0, 2000, 3408, 3875, 4506}; //依次为点击、受欢迎品牌、收藏、购物车、购买的权值
const BYTE type_Vm[VM_NUM] = {0, 4, 2, 3, 1}; //建立访问类型到Vm键值的映射,0：点击，1：购买，2：收藏；3：购物车
BYTE gene_num[VM_NUM-2] = {0}; //个体中染色体需要变异的基因数，即位数

void classify_user_data(char *filename); //把t_alibaba_data.csv文件中的数据进行分类存储
void create_first_generation(ULONG body_num = BODY_NUM); //依据Vm值生成第一代个体
void get_best_Vm(); //获得最好的权值
vector<BRAND_VALUE> get_top_brands_value(const ULONG *Vm_body); //获取价值靠前的品牌列表
bool get_Vm_body(ULONG gen_num, ULONG body_id, ULONG *Vm_body); //获取指定代数指定个体的Vm值
ID_F1 get_f1(ULONG gen_num, ULONG body_id); //获取指定个体的F1值
vector<USER_VALUE> get_top_users_value(const ULONG *Vm_body, const vector<BRAND_VALUE> *pbrands_value); //获取价值靠前的用户列表
USERS_BRANDS get_top_users_brands(const ULONG *Vm_body, const vector<USER_VALUE> *users_value, const vector<BRAND_VALUE> *pbrands_value); //获取的价值靠前的用户可能会买的品牌
float get_precision(const USERS_BRANDS *users_brands); //获取准确率
float get_recall(const USERS_BRANDS *users_brands); //获取召回率
void create_next_generation(ULONG gen_num, vector<ID_F1> *ids_f1); //创建下一代个体
map<ULONG, ULONGS> get_top_Vms(ULONG gen_num, vector<ID_F1> *ids_f1); //获取排在前面的Vm，当然这里Vm的含义为范围值
map<ULONG, ULONGS> get_var_champ(const map<ULONG, ULONGS> *top_Vms); //对排在前面的个体进行基因变异
void cross_champ(const map<ULONG, ULONGS> *var_top_Vms, ULONG gen_num); //进行个体间染色体交叉并产生下一代个体
void write_gen_top_body_f1(ULONG gen_num, ID_F1 id_f1); //把每一代的F1值最高的个体记录下来
void get_best_Vm_ex();
void get_users_buy_rat();
int get_pre_brand_num(ULONG user_id, ULONG sum_brand_num);
vector<USER_VALUE> get_top_users_brands_ex(const ULONG *Vm_body, const vector<BRAND_VALUE> *pbrands_value); //获取的价值靠前的用户可能会买的品牌

bool compare_user_visit_date(const USER_LOG &user_log1, const USER_LOG &user_log2) //按照日期（从小到大）排序的回调函数，参数结构为用户日志
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

bool compare_brand_visit_date(const BRAND_LOG &brand_log1, const BRAND_LOG &brand_log2) //按照日期（从小到大）排序的回调函数，参数结构为品牌日志
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

bool compare_brand_id(const TM_LOG &tm_log1, const TM_LOG &tm_log2) //按照品牌id（从小到大）排序的函数
{
	return tm_log1.brand_id<tm_log2.brand_id;
}

bool compare_brand_id_ex(const USER_LOG &tm_log1, const USER_LOG &tm_log2) //按照品牌id（从小到大）排序的函数,参数是用户日志
{
	return tm_log1.brand_id<tm_log2.brand_id;
}

bool compare_brand_value(const BRAND_VALUE brand_value1, const BRAND_VALUE brand_value2) //按照品牌价值（从大到小）排序
{
	return brand_value1.value>brand_value2.value;
}

bool compare_brand_value_ex(const BRAND_VALUE_EX brand_value_ex1, const BRAND_VALUE_EX brand_value_ex2) //按照品牌价值（从大到小）排序，参数为单个用户的日志
{
	return brand_value_ex1.value >brand_value_ex2.value;
}

bool compare_user_value(const USER_VALUE user_value1, const USER_VALUE user_value2) //按照用户价值（从大到小）排序
{
	return user_value1.val>user_value2.val;
}

bool compare_f1(const ID_F1 &id_f11, const ID_F1 &id_f12) //把处于同一代的所有个体按照F1值从大到小排序
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
	classify_user_data(filename); //先把数据以用户为单位，按照访问日期进行排序
	get_users_buy_rat();
	get_best_Vm(); //获得最好的权值
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
		if(fscanf(file, "%lu,%lu,%u,%u月%u日", &user_id, &brand_id, &type, &mon, &day)!=5)
		{
			fclose(file);
			return;
		}
		user_log.brand_id = brand_id, user_log.type = type, user_log.visit_date.mon = mon, user_log.visit_date.day = day;
		user_logs.push_back(user_log); //存储第一个用户的第一个行为日志

		temp_user_id = user_id;
	}
	while(!feof(file))
	{
		if(fscanf(file, "%lu,%lu,%u,%u月%u日", &user_id, &brand_id, &type, &mon, &day)!=5)
		{
			break;
		}

		if(user_id != temp_user_id)
		{
			users_logs[temp_user_id] = user_logs; //存储中间用户的行为日志
			user_logs.clear();
			temp_user_id = user_id;
		}

		user_log.brand_id = brand_id, user_log.type = type, user_log.visit_date.mon = mon, user_log.visit_date.day = day;
		user_logs.push_back(user_log);
	}
	users_logs[temp_user_id] = user_logs; //存储最后一个用户的行为日志

	fclose(file);

	for(map<ULONG, USER_LOGS>::iterator iter1 = users_logs.begin(); iter1!=users_logs.end(); iter1++)
	{
		sort(iter1->second.begin(), iter1->second.end(), compare_user_visit_date); //按照日期从小到大进行排序

		/*char filename[MAX_PATH] = {0};
		char user_id[10] = {0};
		_itoa(iter1->first, user_id, 10);
		strcpy(filename, "users_logs\\sort_date\\"), strcat(filename, user_id), strcat(filename, ".txt");
		FILE * file = fopen(filename, "w");
		for(USER_LOGS::iterator iter2 = iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
		{
			fprintf(file, "%d,%d,%d,%d月%d日\n", iter1->first, iter2->brand_id, iter2->type, iter2->visit_date.mon, iter2->visit_date.day);
		}
		fclose(file);*/
	}

	for(iter1 = users_logs.begin(); iter1!=users_logs.end(); iter1++)
	{
		USER_LOGS user_logs_pre, user_logs_rea;
		for(USER_LOGS::iterator iter2 = iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
		{
			if(compare_visit_date_ext(iter2->visit_date, divi_date)<=0) //把用户日志以divi_date为界限分成两部分
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

	/***********以下是对品牌id进行分类*******************************/
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
	sort(tm_logs.begin(), tm_logs.end(), compare_brand_id); //按照品牌id从小到大进行排序

	BRAND_LOG brand_log;
	memset(&brand_log, 0, sizeof(BRAND_LOG));
	ULONG temp_brand_id = 0;
	BRAND_LOGS brand_logs;

	if(tm_logs.size()>0)
	{
		brand_log.user_id = tm_logs[0].user_id, brand_log.type = tm_logs[0].type, brand_log.visit_date.mon = tm_logs[0].visit_date.mon, brand_log.visit_date.day = tm_logs[0].visit_date.day;
		brand_logs.push_back(brand_log); //存储第一个品牌的第一个用户日志

		temp_brand_id = tm_logs[0].brand_id;
	}
	for(vector<TM_LOG>::iterator iter = tm_logs.begin()+1; iter!=tm_logs.end(); iter++)
	{
		if(iter->brand_id != temp_brand_id)
		{
			brands_logs[temp_brand_id] = brand_logs; //存储中间品牌的日志
			brand_logs.clear();
			temp_brand_id = iter->brand_id;
		}

		brand_log.user_id = iter->user_id, brand_log.type = iter->type, brand_log.visit_date.mon = iter->visit_date.mon, brand_log.visit_date.day = iter->visit_date.day;
		brand_logs.push_back(brand_log);
	}
	brands_logs[temp_brand_id] = brand_logs; //存储最后一个品牌的日志

	for(map<ULONG, BRAND_LOGS>::iterator iter3 = brands_logs.begin(); iter3!=brands_logs.end(); iter3++)
	{
		sort(iter3->second.begin(), iter3->second.end(), compare_brand_visit_date); //按照日期从小到大进行排序

		/*char filename[MAX_PATH] = {0};
		char brand_id[10] = {0};
		_itoa(iter3->first, brand_id, 10);
		strcpy(filename, "brands_logs\\sort_date\\"), strcat(filename, brand_id), strcat(filename, ".txt");
		FILE * file = fopen(filename, "w");
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			fprintf(file, "%d,%d,%d,%d月%d日\n", iter4->user_id, iter3->first, iter4->type, iter4->visit_date.mon, iter4->visit_date.day);
		}
		fclose(file);*/
	}

	for(iter3 = brands_logs.begin(); iter3!=brands_logs.end(); iter3++)
	{
		BRAND_LOGS brand_logs_pre, brand_logs_rea;
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			if(compare_visit_date_ext(iter4->visit_date, divi_date)<=0) //把品牌日志以divi_date为界限分成两部分
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

	/*file = fopen("test4.txt", "w"); //主要检查读取到内存中数据格式是否正确
	for(iter3 = brands_logs_pre.begin(); iter3!=brands_logs_pre.end(); iter3++)
	{
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			fprintf(file, "%d,%d,%d,%d月%d日\n", iter4->user_id, iter3->first, iter4->type, iter4->visit_date.mon, iter4->visit_date.day);
		}
	}
	fclose(file);

	file = fopen("test5.txt", "w"); //主要检查读取到内存中数据格式是否正确
	for(iter3 = brands_logs_rea.begin(); iter3!=brands_logs_rea.end(); iter3++)
	{
		for(BRAND_LOGS::iterator iter4 = iter3->second.begin(); iter4!=iter3->second.end(); iter4++)
		{
			fprintf(file, "%d,%d,%d,%d月%d日\n", iter4->user_id, iter3->first, iter4->type, iter4->visit_date.mon, iter4->visit_date.day);
		}
	}
	fclose(file);*/

	return;
}

void create_first_generation(ULONG body_num)
{
	for(int i =0; i< VM_NUM; i++)
	{
		Vm[i] *= EXPAND; //先扩大权值
	}

	ULONG Vm_range[VM_NUM-2] = {0};
	for(i = 1; i<VM_NUM-1; i++)
	{
		Vm_range[i-1] = (Vm[i+1] - Vm[i-1])/2;
		Vm_range[i-1] *= RANGE_RATIO; //确定范围大小
		gene_num[i-1] = get_bit_num(Vm_range[i-1]) - NOT_VAR_GENE_NUM;
	}

	for(i = VM_NUM-2; i>0; i--)
	{
		Vm[i] = (Vm[i] + Vm[i-1])/2; //重新调整Vm值，以便加上范围值能直接得到权值
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
			fprintf(file, "%d\n", rand()%Vm_range[j]); //把各个个体值写入文件
		}
		fclose(file);
	}
	return;
}

void get_best_Vm()
{
	create_first_generation(); //生成第一代个体
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
		ids_f1.resize(sqrt(BODY_NUM)); //取前sqrt(BODY_NUM)的目的是让来两两交叉产生两个个体并加上本身刚好为BODY_NUM个

		create_next_generation(gen_num, &ids_f1); //创建下一代个体

		write_gen_top_body_f1(gen_num, ids_f1[0]); //把每一代的F1值最高的个体记录下来
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
		brand_value.visit = false; //初始化为：此品牌没有被访问过
		for(BRAND_LOGS::iterator iter2 = iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
		{
			brand_value.value += Vm_body[type_Vm[iter2->type]]; //品牌的价值，比较合理得算法就是把所有与此品牌有关的行为权值求和
		}
		brands_value.push_back(brand_value);
	}
	sort(brands_value.begin(), brands_value.end(), compare_brand_value); //按照品牌价值从大到小排序

	int size = brands_value.size(); //所有品牌个数
	int top = sqrt(size); //要选取的受欢迎品牌数
	brands_value.resize(top); //取前top个
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

	Vm_body[0] = Vm[0]; //第一个Vm值不变
	for(int i = 1; i< VM_NUM-1; i++)
	{
		ULONG range = 0;
		if(fscanf(file, "%d\n", &range) != 1)//读取的都是Vm的范围值
		{
			printf("\"%s\" content is not correct!\n", filename);
			return false;
		}
		Vm_body[i] = Vm[i] + range; //基础Vm值加上范围值才是某个个体的真正Vm值
	}
	Vm_body[VM_NUM-1] = Vm[VM_NUM-1]; //最后一个Vm值也不变

	fclose(file);
	return true;
}

ID_F1 get_f1(ULONG gen_num, ULONG body_id)
{
	ID_F1 id_f1;

	float f1 = 0.0; // 综合值
	float p = 0.0; //准确率
	float r = 0.0; //召回率

	ULONG Vm_body[VM_NUM] = {0};
	vector<BRAND_VALUE> brands_value; //价值靠前的品牌
	vector<USER_VALUE> users_value; //价值靠前的用户
	USERS_BRANDS users_brands; //最终获取的价值靠前的用户可能会买的品牌

	if(!get_Vm_body(gen_num, body_id, Vm_body)) //获取指定代数指定个体的Vm值
	{
		printf("get_Vm_body failed!\n");
		exit(0);
	}
	brands_value = get_top_brands_value(Vm_body); //获取价值靠前的品牌列表
	//users_value = get_top_users_value(Vm_body, &brands_value); //获取价值靠前的用户列表
	//users_brands = get_top_users_brands(Vm_body, &users_value, &brands_value); //获取的价值靠前的用户可能会买的品牌
	users_value = get_top_users_brands_ex(Vm_body, &brands_value); //获取的价值靠前的用户可能会买的品牌
	users_brands = get_top_users_brands(Vm_body, &users_value, &brands_value); //获取的价值靠前的用户可能会买的品牌

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
			if(find_wel_brands(&brands_value, iter2->brand_id)) //如果用户访问的是受欢迎的品牌，要考虑下面的公式
			{
				sum_val += pow(Vm_body[type_Vm[iter2->type]], 2) / Vm_body[type_Vm[WEL_BRAND]]; //这样做的目的主要考虑用户是否会喜欢受欢迎的品牌
			}
			else
			{
				sum_val += Vm_body[type_Vm[iter2->type]]; //对此用户的所有操作进行累加，暂时不考虑受欢迎品牌
			}
		}
		user_value.val = sum_val;
		users_value.push_back(user_value);
	}
	sort(users_value.begin(), users_value.end(), compare_user_value); //按照品牌价值从大到小排序

	int size = users_value.size(); //所有用户个数
	int top = sqrt(size); //要选取的有价值的用户数
	users_value.resize(187); //取前top个
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
		vector<BRAND_VALUE> brands_value = *pbrands_value; //受欢迎的品牌列表

		USER_LOGS user_logs = users_logs_pre[iter1->user_id];
		sort(user_logs.begin(), user_logs.end(), compare_brand_id_ex); //把品牌从小到大排序，目的是把相同品牌的用户日志放在一起

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
			if(iter2->brand_id != temp_brand_id) //每次遇见和上次不相同的品牌ID就重新计算
			{
				brands_value_ex.push_back(brand_value_ex); //存储此用户的各个品牌价值列表

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

		/*for(vector<BRAND_VALUE>::iterator iter = brands_value.begin(); iter != brands_value.end(); iter ++) //做受欢迎的品牌的推荐
		{
			if(!iter->visit) //如果用户没有访问此品牌，就给用户推荐此品牌，当然此品牌是否能推荐上去，还得看排名
			{
				brand_value_ex.id = iter->id;
				brand_value_ex.value = Vm_body[type_Vm[WEL_BRAND]];
				brands_value_ex.push_back(brand_value_ex);
			}
		}*/
		sort(brands_value_ex.begin(), brands_value_ex.end(), compare_brand_value_ex); //先按照品牌价值从大到小排序
		int size = brands_value_ex.size();
		int top = sqrt(size);
		top = get_pre_brand_num(iter1->user_id, size);
		brands_value_ex.resize(top); //取前top个
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
	map<ULONG, ULONGS> top_Vms = get_top_Vms(gen_num, ids_f1); //获取排在前面的Vm的范围值，即父代

	map<ULONG, ULONGS> var_top_Vms = get_var_champ(&top_Vms); //获取变异后的个体

	cross_champ(&var_top_Vms, gen_num); //进行个体间染色体交叉并产生下一代个体
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
			if(fscanf(file, "%d\n", &range) != 1)//读取的都是Vm的范围值
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
		new_bodys[count] = iter_t->second;  //把排在前面的父个体放在下一代的新个体中
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
			fprintf(file, "%d\n", val); //把各个个体值写入文件
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
	int sum_buy_rea = 0; //第四个月购买总数
	int sum_buy_pre = 0; //前三个月购买总数
	int sum_col_pre = 0; //前三个月收藏总数
	int sum_car_pre = 0; //前三个月购物车总数

	int sum_buy_pre_rea = 0; //前三个月中相同品牌购买量和第四个月相同品牌的购买量的交集
	int sum_col_pre_rea = 0; //前三个月中相同品牌收藏量和第四个月相同品牌的购买量的交集
	int sum_car_pre_rea = 0; //前三个月中相同品牌购物车量和第四个月相同品牌的购买量的交集
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
		brands_value = *pbrands_value; //受欢迎的品牌列表

		user_logs = iter1->second;
		sort(user_logs.begin(), user_logs.end(), compare_brand_id_ex); //把品牌从小到大排序，目的是把相同品牌的用户日志放在一起

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
			if(iter2->brand_id != temp_brand_id) //每次遇见和上次不相同的品牌ID就重新计算
			{
				brands_value_ex.push_back(brand_value_ex); //存储此用户的各个品牌价值列表

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

		sort(brands_value_ex.begin(), brands_value_ex.end(), compare_brand_value_ex); //先按照品牌价值从大到小排序
		int size = brands_value_ex.size();
		int top = sqrt(size);
		top = get_pre_brand_num(iter1->first, size);
		brands_value_ex.resize(top); //取前top个
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
	sort(users_value.begin(), users_value.end(), compare_user_value); //按照品牌价值从大到小排序

	int size = users_value.size(); //所有用户个数
	int top = sqrt(size); //要选取的有价值的用户数
	users_value.resize(187); //取前top个
	return users_value;
}