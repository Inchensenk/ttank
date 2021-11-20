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

//размер консольного поля в символах
#define width 120
#define height 25
//символы для каждого обЪекта на карте
#define field VK_SPACE//символ заполнения поля
#define fbrick 177//символ разрушаемого припятсвия
#define fstone 178//символ неразрушаемого препятсвия

#define base1 5//база первого игрока
#define base2 6//база противника

//координаты певой базы
#define base1X 14
#define base1Y 10
//координаты второй базы
#define base2X 104
#define base2Y 10
//символы для прорисовки танка
#define duloV 179//дуло для прорисовки по вертикали 
#define duloH 205//дуло для прорисовки по горизонтали
#define tankC 219//корпус танка
#define catter 64//гусеница танка



//переменные для выявления индекса обьекта в массиве препятсвий
int nbase1 = -1; //Если значение nbase, не присвоится в цикле с pos, то из - за (- 1) выдаст ошибку, так как индексы массива начинаются с 0
int nbase2 = -1; //Если значение nbase, не присвоится в цикле с pos, то из - за (- 1) выдаст ошибку, так как индексы массива начинаются с 0

// тип данных для карты
typedef char mapHW[height][width];

//COORD Определяет координаты символьной ячейки в буфере экрана консоли.Начало координат системы координат(0, 0) находится в верхней левой ячейке буфера.
//STD_OUTPUT_HANDLE Дескриптор устройства стандартного вывода. Вначале, это - дескриптор активного экранного буфера консоли, CONOUT$. 
//Функция SetConsoleCursorPosition устанавливает положение курсора в консольном окне.
//функция установления положения курсора консоли
void setCurPos(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//функция проверки пересечения 2х прямоугольников
bool IsCross(RECT a, RECT b)
{
    return (a.right >= b.left) && (a.left <= b.right)
        && (a.bottom >= b.top) && (a.top <= b.bottom);
}

//структура для управления картой
struct Tmap
{
    mapHW map;//карта
    //метод Clear() заполняет карту символом field
    void Clear() { memset(map, field, sizeof(map) - 1); }
    //метод SetEnd() ставит в конец двумернорго массива символ терминального нуля
    void SetEnd() { map[height - 1][width - 1] = '\0'; }
    //метод Show() показывает всю карту в консоли
    void Show() { setCurPos(0, 0); SetEnd(); cout << map[0]; }
};

//Tdir тип данных который хранит направление движения танка
//POINT dirInc[] массив который хранит изменение координат при движении по направлению Tdir
enum Tdir { Rup = 0, Rdown, Rleft, Rright };
POINT dirInc[] = { {0,-1},{0,1},{-1,0},{1,0} };

class Ttank {
    int x, y;//положение танка
    int sX, sY;//начальное положение танка
public:
    Tdir dir;//dir куда смотрит танк
    Ttank(int startX, int startY)//начальное положенире танка который по умолчанию смотрит вверх
    {
        dir = Rup; 
        sX = startX; 
        sY = startY; 
        SetToStart();
    }

    void Show(mapHW& map);//отображение танка
    void Move(char w, char s, char a, char d, char fire);//движение танка
    void SetToStart() { x = sX; y = sY; }//возвращение танка в исходное положение
    bool IsHoriz() { return (dir == Rright || dir == Rleft); }// функция IsHoriz() проверяет что танк смотрит горизонтально
    RECT GetRect() { RECT r = { x - 1, y - 1, x + 1, y + 1 }; return r; }// функция GetRect() возвращает область занимаемую танком, для проверки столкновений
};

//Tmatter это тип препятствия, ttStone-камень, ttBrick-кирпич, ttbase1 - база игрока 1, ttbase2 - база игрока 2
enum Tmatter { ttStone, ttBrick, ttbase1, ttbase2};

class Tbrick {
    RECT rct;//rct область которую занимает препятсвие
public:
    bool use;//массив препятсвий статический, поэтому используем use чтобы включать только нужные объекты
    Tmatter tp;//tp тип препятсвия
    Tbrick() { use = 0; tp = ttBrick; }//в конструкторе указываем что по умолчанию препятсвия выключены(use = 0) тип препятсвия по умолчанию кирпич
    void Show(mapHW& map);
    void SetPos(int px, int py) { RECT r = { px - 1, py-1, px+1, py+1 }; rct = r; use = 1; }//помещение препятсвия в нужную точку
    RECT GetRect() { return rct; }
};

class Tpula {
    int x, y;//положение пули
    int speed;//скорость пули в клетках
    Tdir dir;
public:
    bool use;
    Tpula() { use = 0; speed = 3; }//по умолчанию все пули выключены, скорость пули равна 5 клеткам за такт
    void SetPula(int px, int py, Tdir pdir)//метод SetPula() создает пулю в указаной точке, то есть делает ее активной и задает ей направление движения
    {
        x = px; y = py; 
        dir = pdir; 
        use = 1;
    }
    void Move();//перемещение пули
    void Show(mapHW& map) { if (!use) return; map[y][x] = '*'; }//отоброжаем пулю только в том случае если она активна
};



Tmap scr;

#define tankCnt 2
Ttank tank[tankCnt]{ Ttank(1,15), Ttank(118,15) };//массив содержащий координаты начального положения танков
#define brickCnt 300
Tbrick brick[brickCnt];//массив препятсвий
#define pulaCnt 100
Tpula pula[pulaCnt];//массив пуль
// так как пули будут постоянно активироваться и дезактивироваться, нам необходимо выбрать первую свободную пулю

//функция выбора свободной пули
Tpula& GetFreePula()
{
    for (int i = 0; i < pulaCnt; i++)
        if (!pula[i].use)return pula[i];//ищем первый свободный элемент в случае успеха возвращаем его
    return pula[0];//так как функция возвращает ссылку, если ничего не найдено то мы возвращаем первый элемент массива
}
//функция которая проверяет пересечения прямоугольника со всеми танками
Ttank* CheckCrossAnyTank(RECT rct, Ttank* eccept)//RECT rct это область для проверки, Ttank* eccept это танк который не нужно проверять
{
    for (int i = 0; i < tankCnt; i++)//проходим по всем танкам  
        if ((eccept != tank + i) &&//кроме указанного(eccept)
            (IsCross(rct, tank[i].GetRect())))//проверяем пересечение указанного прямоугольника с прямоугольником танка
            return tank + i;//в случае их пересечения возвращаем указатель на танк
    return 0;//в противном случае возвращаем 0
}

//функция которая проверяет пересечение танков с препятсвиями
Tbrick* CheckCrossAnyBrick(RECT rct)
{
    for (int i = 0; i < brickCnt; i++)//проходим по всем препятсвиям
        if (brick[i].use &&//если препятсвие активно
            IsCross(rct, brick[i].GetRect()))//то проверяем на столкновение
            return brick + i;//в случае столкновения возвращаем указатель на обьект
    return 0;//в противном случае возвращаем 0
}

void Ttank::Show(mapHW& map)//&map ссылка на карту на которой будем рисовать
{
    if (IsHoriz())//если танк устал и прилег, рисуем гусеницы горизонтально иначе вертикально
        for (int i = -1; i < 2; map[y - 1][x + i] = map[y + 1][x + i] = catter, i++);
    else
        for (int i = -1; i < 2; map[y + i][x - 1] = map[y + i][x + 1] = catter, i++);
    map[y][x] = tankC;//в центре рисуем корпус танка
    POINT dt = dirInc[dir];//по направлению dir выбираем смещение
    map[y + dt.y][x + dt.x] = IsHoriz() ? duloH : duloV;//в месте смещения рисуем дуло танка. положение дула выбирается в зависимости от положения танка
}

RECT area = { 2,2, width - 3, height - 3 }; //RECT Эта структура определяет прямоугольник.Внутри этой структуры координаты углов. Верхний левый угол определяется двумя координатами left и top. Нижний правый соответственно определяется вдумя оставшимися координатами right и bottom.
//метод реализации движения танка
void Ttank::Move(char w, char s, char a, char d, char fire)
{
    char wsad[4] = { w,s,a,d };
    for (int i = 0; i < 4; i++)
        if (GetKeyState(wsad[i]) < 0) dir = (Tdir)i;
    POINT pt = dirInc[dir];
    Ttank old = *this;
    x += pt.x;
    y += pt.y;
    if (!IsCross(area, GetRect()) ||//если при движении мы вышли за пределы экрана
        (CheckCrossAnyTank(GetRect(), this) != 0) ||//или столкнулись с другим танком
        (CheckCrossAnyBrick(GetRect()) != 0))//или столкнулись с препятсвием
        *this = old;//то мы не перемещаемся
    //реализация стрельбы
    if (GetKeyState(fire) < 0)//при нажатии на кнопку огня
        GetFreePula().SetPula(x + pt.x * 2, y + pt.y * 2, dir);//получаем свободныю пулю в массиве(GetFreePula()) и задаем ей координаты и направления а так же активируем(SetPula(x + pt.x * 2, y + pt.y * 2, dir))
}
void Tbrick::Show(mapHW& map)
{
    if (!use) return;//если препятсвие не используется, то мы его не показываем. ниже рисуем препятсвие и базы
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

RECT areaPula = { 0,0, width - 1, height - 1 };//areaPula- это область в которой могут двигаться пули
void Tpula::Move()//метод движения пули
{
    if (!use) return;//если пуля не используется, то ничего не делаем
    for (int i = 1; i < speed; i++)//при движении пули проверяем каждую клетку на её пути, иначе она будет проходить сквозь танки или препятсвия
    {
        x += dirInc[dir].x;
        y += dirInc[dir].y;// перемещаем пулю на 1 клетку
        RECT rct = { x,y,x,y };//чтобы не перегружать функцию IsCross(), вместо точки передаем ей обоасть
        if (!IsCross(rct, areaPula)) use = 0;//и проверяем, если пуля вышла за экран, то удаляем её

        Tbrick* brick = CheckCrossAnyBrick(rct);//проверяем все препятсвия
        if (brick)use = 0, //если куда-то попали то удаляем пулю
            brick->use = (brick->tp == ttStone);//препятсвие удаляем только если оно не камень(иммено поэтому удобнее возвращать указатель на объект(Tbrick* brick = CheckCrossAnyBrick(rct);), а не булевое значение

        Ttank* tank = CheckCrossAnyTank(rct, 0);//проверяем столкновение пули с танками
        if (tank)use = 0,//если попали по танку, то удаляем пулю
            tank->SetToStart();//а танк возвращаем в исходную точку

        if (!use) return;//если пуля удалено, то выходим из функции
    }
}
//процедура которая будет заполнять карту препятсвиями
void CreateBattleField()
{
    int pos = 0;//счетчик элементов массива препятсвий
    for (int i = 5; i < width - 5; i += 3)
        for (int j = 1; j < height-1; j += 3)//проходим по всей карте с отступом 5 клеток от краев, слева и справа, шаг делаем 3 клетки так как размер препятсвия 3*3
        {
            brick[pos].SetPos(i, j);//в этом месте ставим блок
            //когда генератор препятсвий дойдет до координаты соответсвующейб координате вешины базы, переменной nbase1 присвоится значение индекса обекта базы в массиве препятсвий;
            if (i == base1X && j == base2Y)
            {
                nbase1 = pos;//индекс объекта базы первого игрока в массиве препятсвий
                brick[pos].tp = ttbase1;//база1
                brick[pos].use = 1;
                brick[pos].SetPos(i, j);//база1
            }
            else if (i == base2X && j == base2Y)
            {
                nbase2 = pos;//индекс объекта базы второго игрока в массиве препятсвий
                brick[pos].tp = ttbase2;//присваивание препятсвию тип объекта базы2
                brick[pos].use = 1;
                brick[pos].SetPos(i, j);//база2
            }
            else
            {
                if (rand() % 5 == 0)brick[pos].tp = ttStone;//который с вероятностью 20% будет непробиваемым
                if (rand() % 5 == 0)brick[pos].use = 0;//так же с вероятностью 20% его не будет вообще
            }
            pos++;//переходим к следующему препятсвию в массиве
        }
}

int main()
{
    
    int wins1 = 0;//счетчик попед игрока 1
    int wins2 = 0;//счетчик побед игрока 2

    while(true)
    {
        srand(time(NULL));//инициализация рандома
        for (int i = 0; i < brickCnt; i++)//создает массив из разрушаемых препятсвий для того чтобы при повторной генерации карты не увеличивалось процентное соотношение неразрушаемых препятсвий
        {
            brick[i].tp = BLUE + ttBrick;
        }
        CreateBattleField();//массив из разрушаемых препятсвий заменяется на новый сгенерированный массив из разрушаемых, неразрушаемых препятсвий и баз игроков
        tank[0].SetToStart();//исходное положение танка 1
        tank[1].SetToStart();//исходное положение танка 2
        do
        {
            tank[0].Move('W', 'S', 'A', 'D', VK_SPACE);//первый танк управляется кнопками wsad стреляет клавишей пробел
            tank[1].Move(VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RETURN);//второй танк управляется стрелками и стреляет клавишей Enter, в параметрах указаны коды виртуальных клавиш
            for (int i = 0; i < pulaCnt; pula[i++].Move());//движение пуль делаем после движения танков, так как танк может создать пулю вне экрана, что приведет к ошибке при рисовании пули

            scr.Clear();
            for (int i = 0; i < brickCnt; brick[i++].Show(scr.map));//отоброжение препятсвий и баз
            for (int i = 0; i < tankCnt; tank[i++].Show(scr.map));//отоброжение танков
            for (int i = 0; i < pulaCnt; pula[i++].Show(scr.map));//отоброжение пуль
            scr.Show();

            Sleep(100);//замедление для нормального передвижения танка на экране

            if (brick[nbase1].use == 0)//подсчет побед игрока 2, при уничтожении базы первого игрока, второй игрок получает очко.
            {
                wins2++;
                break;
            }
            else if (brick[nbase2].use == 0)//подсчет побед игрока 1, при уничтожении базы второго игрока, первый игрок получает очко.
            {
                wins1++;
                break;
            }

        } while (GetKeyState(VK_ESCAPE) >= 0);//Функция GetKeyState извлекает данные о состоянии заданной виртуальной клавиши, в даном случае при нажатии на Esc игра перезапускается.

        cout << endl;
        cout << BOLDRED<< "\t\t\t\t\t\t\tResults" << RESET << endl;
        cout << BLUE << "\t\t\t\t\t\t    Player score: " << RESET << wins1 << endl;
        cout << GREEN <<"\t\t\t\t\t\t    Player2 score: " << RESET << wins2 << endl << endl;
        cout << endl;
    }
    

    return 0;
}
