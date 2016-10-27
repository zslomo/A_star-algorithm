/*******************************************************************
*       A*算法实现的复杂环境下最短是路径实现，没有界面，在文件里输入输出
*     需要在文件里用.和x画出地图，x是障碍，XML数据库不会的就用了这个投机
*     取巧的方法，基本所有类的实现和定义我都放在了一个cpp里，方便看
*
*       里面我做了一些优化，就是在启发函数上172-175是4个不同策略的启发函数 
*     优先队列应该用二叉堆实现，这里没有用，还是太懒
*******************************************************************/

#include<iostream>
#include<fstream>
#include<math.h>
#include<time.h>
#include<algorithm>

using namespace std;

const int DISTANCE = 10;
const int direction[8][2] = { { -1,-1 },{ -1,0 },{ -1,1 },{ 0,-1 },{ 0,1 },{ 1,-1 },{ 1,0 },{ 1,1 } };// 方向

enum { LENGTH = 100, WIDE = 100 };//定义数组大小
enum { VIABLE, WALL, INOPEN, INCLOSE, STARTPOINT, DESTINATION };
struct Node
{
	//char perperty;// 属性， 是墙还是起点或是其他
	int    flag; //标志位 0 为可走， 1 为墙壁  2 在openlist  3 在 closelist中 4 为起点 5 为终点
	unsigned int location_x;
	unsigned int location_y;
	unsigned int value_h;
	unsigned int helper_h;
	unsigned int value_g;
	unsigned int value_f;
	unsigned int helper_f;
	Node* parent;
	Node() {
		flag = 0;
		value_h = 0;
		helper_h = 0;
		value_g = 0;
		value_f = 0;
		helper_f = 0;
		parent = NULL;
	}
};

///////////////////////////////////////////////////////////////
// 创建 openlist
//////////////////////////////////////////////////////////////
struct OpenList
{
	Node *opennode;
	OpenList* next;
	OpenList() { next = NULL; };
};


void AddNode2Open(OpenList* openlist, Node* node)
{
	if (openlist == NULL)
	{
		cout << "no data in openlist!" << endl;
		return;
	}
	if (node->flag != STARTPOINT)
	{
		node->flag = INOPEN;
	}
	OpenList* temp = new OpenList;
	temp->next = NULL;
	temp->opennode = node;


	while (openlist->next != NULL)
	{
		if (node->value_f < openlist->next->opennode->value_f)
		{
			OpenList* tempadd = openlist->next;
			temp->next = tempadd;
			openlist->next = temp;
			break;
		}
		else if (node->value_f = openlist->next->opennode->value_f) {
			if (node->helper_f < openlist->next->opennode->helper_f) {
				OpenList* tempadd = openlist->next;
				temp->next = tempadd;
				openlist->next = temp;
				break;
			}
			else
				openlist = openlist->next;
		}
		else
			openlist = openlist->next;
	}
	openlist->next = temp;

}

/////////////////////////////////////////////////////////////
//   创建 closelist
////////////////////////////////////////////////////////////
struct CloseList
{
	Node *closenode;
	CloseList* next;
	CloseList() { next = NULL; };
};
// openlist 此处必须为指针的引用
void AddNode2Close(CloseList* close, OpenList* &open)
{
	if (open == NULL)
	{
		cout << "no data in openlist!" << endl;
		return;
	}
	if (open->opennode->flag != STARTPOINT)
		open->opennode->flag = INCLOSE;

	if (close->closenode == NULL)
	{
		close->closenode = open->opennode;
		OpenList* tempopen = open;
		open = open->next;
		delete tempopen;
		return;
	}
	while (close->next != NULL)
		close = close->next;

	CloseList* temp = new CloseList;
	temp->closenode = open->opennode;
	temp->next = NULL;
	close->next = temp;

	OpenList* tempopen = open;
	open = open->next;
	delete tempopen;
}
void RemoveFromOpenlist(OpenList *&openlist, Node *node) {
	if (openlist->opennode == node) {
		OpenList* tempopen = openlist;
		openlist = openlist->next;
		delete tempopen;
	}
	else {
		OpenList* p = openlist;
		while (p->next->opennode != node) p = p->next;
		OpenList* tempopen = p->next;
		p->next = p->next->next;
		delete tempopen;
	}
}

////////////////////////////////////////////////////////
//   查找类
///////////////////////////////////////////////////////
class AStartFindPath
{
public:
	Node m_node[LENGTH][WIDE];
	AStartFindPath();

	virtual ~AStartFindPath() {};

	void InitNodeMap(char aa[][WIDE], OpenList *open);
	void FindDestinnation(OpenList* open, CloseList* close, char aa[][WIDE]);
	//OpenList* FindMinInOpen(OpenList* open);
	bool Insert2OpenList(OpenList*, int sx, int sy, int x, int y);
	bool IsInOpenList(OpenList*, int x, int y);
	bool IsInCloseList(OpenList*, int x, int y);
	void IsChangeParent(OpenList*, int x, int y);
	bool IsAviable(OpenList*, int x, int y);
	unsigned int DistanceManhattan(int sx, int sy, int d_x, int d_y, int x, int y);
	unsigned int DistanceCross(int sx, int sy, int d_x, int d_y, int x, int y);
	unsigned int DistanceChebyshev(int sx, int sy, int d_x, int d_y, int x, int y);
	unsigned int DistanceStraight(int sx, int sy, int d_x, int d_y, int x, int y);
private:
	unsigned int steps;
	int startpoint_x;
	int startpoint_y;
	int endpoint_x;
	int endpoint_y;

};

AStartFindPath::AStartFindPath()
{
	steps = 0;
	startpoint_x = -1;
	startpoint_y = -1;
	endpoint_y = -1;
	endpoint_x = -1;
}
void AStartFindPath::FindDestinnation(OpenList* open, CloseList* close, char aa[][WIDE])
{
	Insert2OpenList(open, startpoint_x, startpoint_y, startpoint_x, startpoint_y);// 起点
	AddNode2Close(close, open);// 起点放到 close中
							   //OpenList* temp=FindMinInOpen(open);
	while (!Insert2OpenList(open, startpoint_x, startpoint_y, open->opennode->location_x, open->opennode->location_y))
	{
		AddNode2Close(close, open);
		if (open == NULL)
		{
			cout << "未找到出口！地图有误" << endl;
			return;
		}
	}
	Node *tempnode = &m_node[endpoint_x][endpoint_y];
	while (tempnode->parent->flag != STARTPOINT)
	{
		tempnode = tempnode->parent;
		aa[tempnode->location_x][tempnode->location_y] = '@';
	}
	m_node;
}
//// 在openlist中找到最小的 f值  节点
//OpenList* AStartFindPath::FindMinInOpen(OpenList* open)
//{
//	return open;
//}
//////////////////////////////////////////////////////////////////////////
//  将临近的节点加入 openlist中
//				0      1      2  
//				3      S      4
//				5      6      7
/////////////////////////////////////////////////////////////////////////////
bool AStartFindPath::Insert2OpenList(OpenList* open, int sx, int sy, int center_x, int center_y)
{
	int i = 0;
	for (; i < 8; i++)
	{
		int new_x = center_x + direction[i][0];
		int new_y = center_y + direction[i][1];

		if (new_x >= 0 && new_y >= 0 && new_x < LENGTH &&
			new_y < WIDE &&
			IsAviable(open, new_x, new_y))// 0
		{
			if (m_node[new_x][new_y].flag == DESTINATION)
			{
				m_node[new_x][new_y].parent = &m_node[center_x][center_y];
				return true;
			}
			m_node[new_x][new_y].flag = INOPEN;
			m_node[new_x][new_y].parent = &m_node[center_x][center_y];
			m_node[new_x][new_y].value_h =
				DistanceChebyshev(sx, sy, endpoint_x, endpoint_y, new_x, new_y);

			m_node[new_x][new_y].helper_h =
				DistanceCross(sx, sy, endpoint_x, endpoint_y, new_x, new_y);
			if (0 == i || 2 == i || 5 == i || 7 == i)
				m_node[new_x][new_y].value_g = m_node[center_x][center_y].value_g + 14;
			else
				m_node[new_x][new_y].value_g = m_node[center_x][center_y].value_g + 10;

			m_node[new_x][new_y].value_f = m_node[new_x][new_y].value_g + m_node[new_x][new_y].value_h;
			m_node[new_x][new_y].helper_f = m_node[new_x][new_y].value_g + m_node[new_x][new_y].helper_h;

			AddNode2Open(open, &m_node[new_x][new_y]);// 加入到 openlist中
		}
	}
	IsChangeParent(open, center_x, center_y);

	return false;
}
// 是否有更好的路径
void AStartFindPath::IsChangeParent(OpenList* open, int center_x, int center_y)
{
	int i = 0;
	for (; i < 8; i++)
	{
		int new_x = center_x + direction[i][0];
		int new_y = center_y + direction[i][1];
		if (new_x >= 0 && new_y >= 0 && new_x < LENGTH &&
			new_y<WIDE &&
			IsInOpenList(open, new_x, new_y))// 0
		{

			if (0 == i || 2 == i || 5 == i || 7 == i)
			{
				if (m_node[new_x][new_y].value_g >  m_node[center_x][center_y].value_g + 14)
				{
					m_node[new_x][new_y].parent = &m_node[center_x][center_y];
					m_node[new_x][new_y].value_g = m_node[center_x][center_y].value_g + 14;
					m_node[new_x][new_y].value_f = m_node[new_x][new_y].value_g + m_node[new_x][new_y].value_h;
					m_node[new_x][new_y].helper_f = m_node[new_x][new_y].value_g + m_node[new_x][new_y].helper_h;
				}
			}
			else
			{
				if (m_node[new_x][new_y].value_g > m_node[center_x][center_y].value_g + 10)
				{
					m_node[new_x][new_y].parent = &m_node[center_x][center_y];
					m_node[new_x][new_y].value_g = m_node[center_x][center_y].value_g + 10;
					m_node[new_x][new_y].value_f = m_node[new_x][new_y].value_g + m_node[new_x][new_y].value_h;
					m_node[new_x][new_y].helper_f = m_node[new_x][new_y].value_g + m_node[new_x][new_y].helper_h;
				}
			}
			RemoveFromOpenlist(open, &m_node[new_x][new_y]);
			AddNode2Open(open, &m_node[new_x][new_y]);
		}	
	}
}

bool AStartFindPath::IsAviable(OpenList* open, int x, int y)
{
	if (IsInOpenList(open, x, y))
		return false;
	if (IsInCloseList(open, x, y))
		return false;
	if (m_node[x][y].flag == WALL)
		return false;
	else
		return true;
}
bool AStartFindPath::IsInOpenList(OpenList* openlist, int x, int y)
{
	if (m_node[x][y].flag == INOPEN)
		return true;
	else
		return false;
}

bool AStartFindPath::IsInCloseList(OpenList* openlist, int x, int y)
{
	if (m_node[x][y].flag == INCLOSE || m_node[x][y].flag == STARTPOINT)
		return true;
	else
		return false;
}
//显示地图
void DisplayMap(char aa[][WIDE])
{
	for (int i = 0; i < LENGTH; i++)
	{
		for (int j = 0; j < WIDE; j++)
			cout << aa[i][j];
		cout << endl;
	}
}
unsigned int AStartFindPath::DistanceStraight(int sx, int sy, int d_x, int d_y, int x, int y) {

	unsigned int a = (d_x - x)*(d_x - x);
	unsigned int b = (d_y - y)*(d_y - y);
	return sqrt(a + b)*DISTANCE;
}
unsigned int AStartFindPath::DistanceCross(int sx, int sy, int d_x, int d_y, int x, int y) {
	int dx1 = x - d_x;
	int dy1 = y - d_y;
	int dx2 = sx - d_x;
	int dy2 = sy - d_y;
	unsigned int temp = abs(dx1*dy2 - dx2*dy1)*DISTANCE;
	return temp;
}
unsigned int AStartFindPath::DistanceChebyshev(int sx, int sy, int d_x, int d_y, int x, int y) {
	int dx = x - d_x;
	int dy = y - d_y;
	return (dx + dy)*DISTANCE - 6*min(dx, dy);
}
unsigned int AStartFindPath::DistanceManhattan(int sx, int sy, int d_x, int d_y, int x, int y)
{


	unsigned int temp = (abs(d_x - x) + abs(d_y - y))*DISTANCE;
	return temp;
}
//初始化 node
void AStartFindPath::InitNodeMap(char aa[][WIDE], OpenList * openlist)
{
	ifstream fin;
	fin.open("D://datatest1.txt");
	if (!fin.is_open())
	{
		cout << "Failed to open file!" << endl;
		return;
	}
	for (int i = 0; i < LENGTH; i++)
	{
		for (int j = 0; j < WIDE; j++)
		{
			fin >> aa[i][j];
			m_node[i][j].location_x = i;
			m_node[i][j].location_y = j;
			m_node[i][j].parent = NULL;
			switch (aa[i][j])
			{
			case '.':
				m_node[i][j].flag = VIABLE;
				break;
			case 'x':
				m_node[i][j].flag = WALL;
				break;
			case 's':
				m_node[i][j].flag = STARTPOINT;
				openlist->next = NULL;
				openlist->opennode = &m_node[i][j];//  将起点放到 OPenList中		
				startpoint_x = i;
				startpoint_y = j;
				break;
			case 'd':
				m_node[i][j].flag = DESTINATION;
				endpoint_x = i;
				endpoint_y = j;
				break;
			}
		}
		fin.get();
	}
	//cout<<aa[startpoint_x][startpoint_y]<<endl;
	fin.close();


}

int main(int argc, char *argv[])
{
	int beginTime = clock();
	ofstream outfile("d://a.txt");
	AStartFindPath findpath;
	OpenList* openlist = new OpenList;
	CloseList* closelist = new CloseList;
	//closelist=NULL;
	const OpenList* HEADOPEN = openlist;
	const CloseList* HEADCLOSE = closelist;
	closelist->closenode = NULL;

	char aa[LENGTH][WIDE] = { 0 };

	findpath.InitNodeMap(aa, openlist);


	findpath.FindDestinnation(openlist, closelist, aa);
	int endTime = clock();
	cout <<"用时 "<< endTime - beginTime << "ms" << endl;
	for (int i = 0; i < LENGTH; i++)
	{
		for (int j = 0; j < WIDE; j++)
		{
			outfile << aa[i][j];
		}
		outfile << endl;
	}
	system("pause");
	return 0;
}