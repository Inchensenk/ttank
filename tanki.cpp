#include <iostream>
#include <cstring>
#include <windows.h>
#include <ctime>

using namespace std;

#define RESET   "\033[0m"

#define BLACK   "\033[30m"     
#define RED     "\033[31m"     
#define GREEN   "\033[32m"      
#define YELLOW  "\033[33m"      
#define BLUE    "\033[34m"      
#define MAGENTA "\033[35m"      
#define CYAN    "\033[36m"      
#define WHITE   "\033[37m"    

#define BOLDBLACK   "\033[1m\033[30m"      
#define BOLDRED     "\033[1m\033[31m"     
#define BOLDGREEN   "\033[1m\033[32m"      
#define BOLDYELLOW  "\033[1m\033[33m"      
#define BOLDBLUE    "\033[1m\033[34m"      
#define BOLDMAGENTA "\033[1m\033[35m"      
#define BOLDCYAN    "\033[1m\033[36m"      
#define BOLDWHITE   "\033[1m\033[37m"      

//������ ����������� ���� � ��������
#define width 120
#define height 25
//������� ��� ������� ������� �� �����
#define field VK_SPACE//������ ���������� ����
#define fbrick 177//������ ������������ ����������
#define fstone 178//������ �������������� ����������

#define base1 5//���� ������� ������
#define base2 6//���� ����������

//���������� ����� ����
#define base1X 14
#define base1Y 10
//���������� ������ ����
#define base2X 104
#define base2Y 10
//������� ��� ���������� �����
#define duloV 179//���� ��� ���������� �� ��������� 
#define duloH 205//���� ��� ���������� �� �����������
#define tankC 219//������ �����
#define catter 64//�������� �����



//���������� ��� ��������� ������� ������� � ������� ����������
int nbase1 = -1; //���� �������� nbase, �� ���������� � ����� � pos, �� �� - �� (- 1) ������ ������, ��� ��� ������� ������� ���������� � 0
int nbase2 = -1; //���� �������� nbase, �� ���������� � ����� � pos, �� �� - �� (- 1) ������ ������, ��� ��� ������� ������� ���������� � 0

// ��� ������ ��� �����
typedef char mapHW[height][width];

//COORD ���������� ���������� ���������� ������ � ������ ������ �������.������ ��������� ������� ���������(0, 0) ��������� � ������� ����� ������ ������.
//STD_OUTPUT_HANDLE ���������� ���������� ������������ ������. �������, ��� - ���������� ��������� ��������� ������ �������, CONOUT$. 
//������� SetConsoleCursorPosition ������������� ��������� ������� � ���������� ����.
//������� ������������ ��������� ������� �������
void setCurPos(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//������� �������� ����������� 2� ���������������
bool IsCross(RECT a, RECT b)
{
    return (a.right >= b.left) && (a.left <= b.right)
        && (a.bottom >= b.top) && (a.top <= b.bottom);
}

//��������� ��� ���������� ������
struct Tmap
{
    mapHW map;//�����
    //����� Clear() ��������� ����� �������� field
    void Clear() { memset(map, field, sizeof(map) - 1); }
    //����� SetEnd() ������ � ����� ����������� ������� ������ ������������� ����
    void SetEnd() { map[height - 1][width - 1] = '\0'; }
    //����� Show() ���������� ��� ����� � �������
    void Show() { setCurPos(0, 0); SetEnd(); cout << map[0]; }
};

//Tdir ��� ������ ������� ������ ����������� �������� �����
//POINT dirInc[] ������ ������� ������ ��������� ��������� ��� �������� �� ����������� Tdir
enum Tdir { Rup = 0, Rdown, Rleft, Rright };
POINT dirInc[] = { {0,-1},{0,1},{-1,0},{1,0} };

class Ttank {
    int x, y;//��������� �����
    int sX, sY;//��������� ��������� �����
public:
    Tdir dir;//dir ���� ������� ����
    Ttank(int startX, int startY)//��������� ���������� ����� ������� �� ��������� ������� �����
    {
        dir = Rup; 
        sX = startX; 
        sY = startY; 
        SetToStart();
    }

    void Show(mapHW& map);//����������� �����
    void Move(char w, char s, char a, char d, char fire);//�������� �����
    void SetToStart() { x = sX; y = sY; }//����������� ����� � �������� ���������
    bool IsHoriz() { return (dir == Rright || dir == Rleft); }// ������� IsHoriz() ��������� ��� ���� ������� �������������
    RECT GetRect() { RECT r = { x - 1, y - 1, x + 1, y + 1 }; return r; }// ������� GetRect() ���������� ������� ���������� ������, ��� �������� ������������
};

//Tmatter ��� ��� �����������, ttStone-������, ttBrick-������, ttbase1 - ���� ������ 1, ttbase2 - ���� ������ 2
enum Tmatter { ttStone, ttBrick, ttbase1, ttbase2};

class Tbrick {
    RECT rct;//rct ������� ������� �������� ����������
public:
    bool use;//������ ���������� �����������, ������� ���������� use ����� �������� ������ ������ �������
    Tmatter tp;//tp ��� ����������
    Tbrick() { use = 0; tp = ttBrick; }//� ������������ ��������� ��� �� ��������� ���������� ���������(use = 0) ��� ���������� �� ��������� ������
    void Show(mapHW& map);
    void SetPos(int px, int py) { RECT r = { px - 1, py-1, px+1, py+1 }; rct = r; use = 1; }//��������� ���������� � ������ �����
    RECT GetRect() { return rct; }
};

class Tpula {
    int x, y;//��������� ����
    int speed;//�������� ���� � �������
    Tdir dir;
public:
    bool use;
    Tpula() { use = 0; speed = 3; }//�� ��������� ��� ���� ���������, �������� ���� ����� 5 ������� �� ����
    void SetPula(int px, int py, Tdir pdir)//����� SetPula() ������� ���� � �������� �����, �� ���� ������ �� �������� � ������ �� ����������� ��������
    {
        x = px; y = py; 
        dir = pdir; 
        use = 1;
    }
    void Move();//����������� ����
    void Show(mapHW& map) { if (!use) return; map[y][x] = '*'; }//���������� ���� ������ � ��� ������ ���� ��� �������
};



Tmap scr;

#define tankCnt 2
Ttank tank[tankCnt]{ Ttank(1,15), Ttank(118,15) };//������ ���������� ���������� ���������� ��������� ������
#define brickCnt 300
Tbrick brick[brickCnt];//������ ����������
#define pulaCnt 100
Tpula pula[pulaCnt];//������ ����
// ��� ��� ���� ����� ��������� �������������� � �����������������, ��� ���������� ������� ������ ��������� ����

//������� ������ ��������� ����
Tpula& GetFreePula()
{
    for (int i = 0; i < pulaCnt; i++)
        if (!pula[i].use)return pula[i];//���� ������ ��������� ������� � ������ ������ ���������� ���
    return pula[0];//��� ��� ������� ���������� ������, ���� ������ �� ������� �� �� ���������� ������ ������� �������
}
//������� ������� ��������� ����������� �������������� �� ����� �������
Ttank* CheckCrossAnyTank(RECT rct, Ttank* eccept)//RECT rct ��� ������� ��� ��������, Ttank* eccept ��� ���� ������� �� ����� ���������
{
    for (int i = 0; i < tankCnt; i++)//�������� �� ���� ������  
        if ((eccept != tank + i) &&//����� ����������(eccept)
            (IsCross(rct, tank[i].GetRect())))//��������� ����������� ���������� �������������� � ��������������� �����
            return tank + i;//� ������ �� ����������� ���������� ��������� �� ����
    return 0;//� ��������� ������ ���������� 0
}

//������� ������� ��������� ����������� ������ � ������������
Tbrick* CheckCrossAnyBrick(RECT rct)
{
    for (int i = 0; i < brickCnt; i++)//�������� �� ���� �����������
        if (brick[i].use &&//���� ���������� �������
            IsCross(rct, brick[i].GetRect()))//�� ��������� �� ������������
            return brick + i;//� ������ ������������ ���������� ��������� �� ������
    return 0;//� ��������� ������ ���������� 0
}

void Ttank::Show(mapHW& map)//&map ������ �� ����� �� ������� ����� ��������
{
    if (IsHoriz())//���� ���� ����� � ������, ������ �������� ������������� ����� �����������
        for (int i = -1; i < 2; map[y - 1][x + i] = map[y + 1][x + i] = catter, i++);
    else
        for (int i = -1; i < 2; map[y + i][x - 1] = map[y + i][x + 1] = catter, i++);
    map[y][x] = tankC;//� ������ ������ ������ �����
    POINT dt = dirInc[dir];//�� ����������� dir �������� ��������
    map[y + dt.y][x + dt.x] = IsHoriz() ? duloH : duloV;//� ����� �������� ������ ���� �����. ��������� ���� ���������� � ����������� �� ��������� �����
}

RECT area = { 2,2, width - 3, height - 3 }; //RECT ��� ��������� ���������� �������������.������ ���� ��������� ���������� �����. ������� ����� ���� ������������ ����� ������������ left � top. ������ ������ �������������� ������������ ����� ����������� ������������ right � bottom.
//����� ���������� �������� �����
void Ttank::Move(char w, char s, char a, char d, char fire)
{
    char wsad[4] = { w,s,a,d };
    for (int i = 0; i < 4; i++)
        if (GetKeyState(wsad[i]) < 0) dir = (Tdir)i;
    POINT pt = dirInc[dir];
    Ttank old = *this;
    x += pt.x;
    y += pt.y;
    if (!IsCross(area, GetRect()) ||//���� ��� �������� �� ����� �� ������� ������
        (CheckCrossAnyTank(GetRect(), this) != 0) ||//��� ����������� � ������ ������
        (CheckCrossAnyBrick(GetRect()) != 0))//��� ����������� � �����������
        *this = old;//�� �� �� ������������
    //���������� ��������
    if (GetKeyState(fire) < 0)//��� ������� �� ������ ����
        GetFreePula().SetPula(x + pt.x * 2, y + pt.y * 2, dir);//�������� ��������� ���� � �������(GetFreePula()) � ������ �� ���������� � ����������� � ��� �� ����������(SetPula(x + pt.x * 2, y + pt.y * 2, dir))
}
void Tbrick::Show(mapHW& map)
{
    if (!use) return;//���� ���������� �� ������������, �� �� ��� �� ����������. ���� ������ ���������� � ����
    for (int i = rct.left; i <= rct.right; i++)
        for (int j = rct.top; j <= rct.bottom; j++)
            if (tp == ttBrick)
                map[j][i] = fbrick;
            else if (tp==ttStone)
                map[j][i] = fstone;
            else if (tp==ttbase1)
                map[j][i] = base1;
            else if (tp == ttbase2)
                map[j][i] = base2;
}

RECT areaPula = { 0,0, width - 1, height - 1 };//areaPula- ��� ������� � ������� ����� ��������� ����
void Tpula::Move()//����� �������� ����
{
    if (!use) return;//���� ���� �� ������������, �� ������ �� ������
    for (int i = 1; i < speed; i++)//��� �������� ���� ��������� ������ ������ �� � ����, ����� ��� ����� ��������� ������ ����� ��� ����������
    {
        x += dirInc[dir].x;
        y += dirInc[dir].y;// ���������� ���� �� 1 ������
        RECT rct = { x,y,x,y };//����� �� ����������� ������� IsCross(), ������ ����� �������� �� �������
        if (!IsCross(rct, areaPula)) use = 0;//� ���������, ���� ���� ����� �� �����, �� ������� �

        Tbrick* brick = CheckCrossAnyBrick(rct);//��������� ��� ����������
        if (brick)use = 0, //���� ����-�� ������ �� ������� ����
            brick->use = (brick->tp == ttStone);//���������� ������� ������ ���� ��� �� ������(������ ������� ������� ���������� ��������� �� ������(Tbrick* brick = CheckCrossAnyBrick(rct);), � �� ������� ��������

        Ttank* tank = CheckCrossAnyTank(rct, 0);//��������� ������������ ���� � �������
        if (tank)use = 0,//���� ������ �� �����, �� ������� ����
            tank->SetToStart();//� ���� ���������� � �������� �����

        if (!use) return;//���� ���� �������, �� ������� �� �������
    }
}
//��������� ������� ����� ��������� ����� ������������
void CreateBattleField()
{
    int pos = 0;//������� ��������� ������� ����������
    for (int i = 5; i < width - 5; i += 3)
        for (int j = 1; j < height-1; j += 3)//�������� �� ���� ����� � �������� 5 ������ �� �����, ����� � ������, ��� ������ 3 ������ ��� ��� ������ ���������� 3*3
        {
            brick[pos].SetPos(i, j);//� ���� ����� ������ ����
            //����� ��������� ���������� ������ �� ���������� ��������������� ���������� ������ ����, ���������� nbase1 ���������� �������� ������� ������ ���� � ������� ����������;
            if (i == base1X && j == base2Y)
            {
                nbase1 = pos;//������ ������� ���� ������� ������ � ������� ����������
                brick[pos].tp = ttbase1;//����1
                brick[pos].use = 1;
                brick[pos].SetPos(i, j);//����1
            }
            else if (i == base2X && j == base2Y)
            {
                nbase2 = pos;//������ ������� ���� ������� ������ � ������� ����������
                brick[pos].tp = ttbase2;//������������ ���������� ��� ������� ����2
                brick[pos].use = 1;
                brick[pos].SetPos(i, j);//����2
            }
            else
            {
                if (rand() % 5 == 0)brick[pos].tp = ttStone;//������� � ������������ 20% ����� �������������
                if (rand() % 5 == 0)brick[pos].use = 0;//��� �� � ������������ 20% ��� �� ����� ������
            }
            pos++;//��������� � ���������� ���������� � �������
        }
}

int main()
{
    
    int wins1 = 0;//������� ����� ������ 1
    int wins2 = 0;//������� ����� ������ 2

    while(true)
    {
        srand(time(NULL));//������������� �������
        for (int i = 0; i < brickCnt; i++)//������� ������ �� ����������� ���������� ��� ���� ����� ��� ��������� ��������� ����� �� ������������� ���������� ����������� ������������� ����������
        {
            brick[i].tp = BLUE + ttBrick;
        }
        CreateBattleField();//������ �� ����������� ���������� ���������� �� ����� ��������������� ������ �� �����������, ������������� ���������� � ��� �������
        tank[0].SetToStart();//�������� ��������� ����� 1
        tank[1].SetToStart();//�������� ��������� ����� 2
        do
        {
            tank[0].Move('W', 'S', 'A', 'D', VK_SPACE);//������ ���� ����������� �������� wsad �������� �������� ������
            tank[1].Move(VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RETURN);//������ ���� ����������� ��������� � �������� �������� Enter, � ���������� ������� ���� ����������� ������
            for (int i = 0; i < pulaCnt; pula[i++].Move());//�������� ���� ������ ����� �������� ������, ��� ��� ���� ����� ������� ���� ��� ������, ��� �������� � ������ ��� ��������� ����

            scr.Clear();
            for (int i = 0; i < brickCnt; brick[i++].Show(scr.map));//����������� ���������� � ���
            for (int i = 0; i < tankCnt; tank[i++].Show(scr.map));//����������� ������
            for (int i = 0; i < pulaCnt; pula[i++].Show(scr.map));//����������� ����
            scr.Show();

            Sleep(100);//���������� ��� ����������� ������������ ����� �� ������

            if (brick[nbase1].use == 0)//������� ����� ������ 2, ��� ����������� ���� ������� ������, ������ ����� �������� ����.
            {
                wins2++;
                break;
            }
            else if (brick[nbase2].use == 0)//������� ����� ������ 1, ��� ����������� ���� ������� ������, ������ ����� �������� ����.
            {
                wins1++;
                break;
            }

        } while (GetKeyState(VK_ESCAPE) >= 0);//������� GetKeyState ��������� ������ � ��������� �������� ����������� �������, � ����� ������ ��� ������� �� Esc ���� ���������������.

        cout << endl;
        cout << BOLDRED<< "\t\t\t\t\t\t\tResults" << RESET << endl;
        cout << BLUE << "\t\t\t\t\t\t    Player score: " << RESET << wins1 << endl;
        cout << GREEN <<"\t\t\t\t\t\t    Player2 score: " << RESET << wins2 << endl << endl;
        cout << endl;
    }
    

    return 0;
}
