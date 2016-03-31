#include <iostream>
#include <cstdio>
#include <vector>
#include <queue>
using namespace std;

const int HARD_OJAMA = -2;
const int NORMAL_OJAMA = -1;
const int EMPTY = 0;
//�F�̂������ʂ̋ʂ�, 1�ȏ�M�ȉ��̐����ŕ\�����

const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0}; //��, �E, ��, ��
const double DINF = INT_MAX;

class Point
{
public:
	int x, y;
	Point()
	{
		x = y = -1;
	}
	Point(int _x, int _y)
	{
		x = _x; y = _y;
	}
	//���W���w�肵��������1�}�X������
	void Move(int dir)
	{
		x += dx[dir]; y += dy[dir];
	}
	bool operator == (const Point &p) const { return x == p.x && y == p.y; }
};

class Cell
{
public:
	int kind;
	Cell()
	{
		kind = EMPTY;
	}
	Cell(int k)
	{
		kind = k;
	}
	static Cell Input()
	{
		int k;
		//���͎��s���̏I������
		if (!(cin >> k)) exit(0);
		return Cell(k);
	}
	bool IsOjama() const { return kind < 0; }
	bool IsHardOjama() const { return kind == -2; }
	bool IsNormalOjama() const { return kind == -1; }
	bool IsEmpty() const { return kind == 0; }
	bool IsColorful() const { return kind > 0; }
};

class OjamaCalculator
{
public:
	int weakness; //������܂���̉����ꂽ��
	int ojamaErasure; //������܂���������
	int colorfulErasure; //�F�t���ʂ���������
	OjamaCalculator()
	{
		weakness = ojamaErasure = colorfulErasure = 0;
	}
	OjamaCalculator(int _hardOjama, int _normalOjama, int _colorfulBall)
	{
		weakness = _hardOjama; ojamaErasure = _normalOjama; colorfulErasure = _colorfulBall;
	}
	bool IsHard() const
	{
		return colorfulErasure >= 35;
	}
	int Calculate() const
	{
		//TODO ����
		int ojamas = ojamaErasure + weakness;
		double k = 0.015;
		double l = 0.15;
		if (IsHard())
		{
			k *= 0.6;
			l *= 0.6;
		}
		return (int)(colorfulErasure*colorfulErasure*k + ojamas*ojamas*l);
	}
};

class State
{
public:
	int W, H, N, M;
	//���W(x,y)�̏���field[x][y]�ɓ����Ă���
	//field[y][x]�ł͂Ȃ��̂Œ���!
	vector<vector<Cell> > field;
	//rain[x]�ɂ�, ��x�ɍ~��\��̋ʂ�\���������~�鏇�Ԃɋl�߂�
	vector<queue<int> > rain;

	State()
	{
		W = H = N = M = 0;
	}

	static State Input(int w, int h, int n, int m)
	{
		State res;
		res.W = w; res.H = h; res.N = n; res.M = m;
		res.field.resize(w);
		res.rain.resize(w);
		for (int x = 0; x < w; x++) res.field[x].reserve(h);
		for (int x = 0; x < w; x++)
		{
			int cnt;
			cin >> cnt;
			for (int i = 0; i < cnt; i++)
			{
				int ojama;
				cin >> ojama;
				res.rain[x].push(ojama);
			}
		}
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				res.field[x].push_back(Cell::Input());
			}
		}
		return res;
	}

	bool IsOutside(const Point &p) const { return p.x < 0 || W <= p.x || p.y < 0 || H <= p.y; }

private:
	void getLump(vector<Point> &ret, const Point &pos, int kind, vector< vector<bool> > &used)
		const
	{
		ret.push_back(pos);
		for (int i = 0; i < 4; i++)
		{
			Point next(pos);
			next.Move(i);
			if (IsOutside(next) || field[next.x][next.y].kind != kind || used[next.x][next.y])
				continue;
			used[next.x][next.y] = true;
			getLump(ret, next, kind, used);
		}
	}

public:
	//�w�肵�����W�̋ʂƊ��S�ɓ�����ނ̋ʂ̂����A�q�����Ă�����̂̍��W��Ԃ�(�w�肵�����W���܂�)
	//used�͊��ɒ��ׂ����W�̂Ƃ����true������
	vector<Point> GetLump(const Point &pos, vector< vector<bool> > used) const
	{
		vector<Point> res;
		if (used[pos.x][pos.y]) return res;
		used[pos.x][pos.y] = true;
		getLump(res, pos, field[pos.x][pos.y].kind, used);
		return res;
	}
	//used�������ō���(�g���񂹂�Ƃ��͊O���Ő錾���Ďg���񂷂ق�������?)
	vector<Point> GetLump(const Point &pos) const
	{
		vector< vector<bool> > used(W);
		for (int i = 0; i < W; i++) used[i].resize(H, false);
		return GetLump(pos, used);
	}

	//�ʂ̗������鏈����Rain()�ł܂Ƃ߂čs����悤�ɂ��邽��, �����ꂽ�ʂ̂Ƃ���͋�ɂ��Ă���
	//�Ȃ�, ���ۂ̃Q�[���ɂ͋�̃}�X�͑��݂��Ȃ��̂Œ���!
	void Erase(const Point &pos)
	{
		field[pos.x][pos.y].kind = EMPTY;
	}
	void AttackOjama(const Point &pos)
	{
		if (!field[pos.x][pos.y].IsOjama()) return;
		field[pos.x][pos.y].kind++;
	}

	//�F�t���ʂ̉򂪏��������Ɋ������܂�邨����܂𐔂���
	OjamaCalculator CountOjamas(const vector<Point> &colorfulLump) const
	{
		OjamaCalculator res;
		vector< vector<int> > cntNext(W);
		for (int i = 0; i < W; i++) cntNext[i].resize(H, 0);
		res.colorfulErasure = colorfulLump.size();
		for (int i = 0; i < res.colorfulErasure; i++)
		{
			if (!field[colorfulLump[i].x][colorfulLump[i].y].IsColorful()) continue;
			for (int dir = 0; dir < 4; dir++)
			{
				Point next(colorfulLump[i]);
				next.Move(dir);
				if (IsOutside(next) || !field[next.x][next.y].IsOjama()) continue;
				cntNext[next.x][next.y]++;
				//������܂�������Ȃ�
				if (cntNext[next.x][next.y] == -field[next.x][next.y].kind)
					res.ojamaErasure++;
				//������܂���̉������Ȃ�
				if (field[next.x][next.y].IsHardOjama() && cntNext[next.x][next.y] == 1)
					res.weakness++;
			}
		}
		return res;
	}

	void Erase(const vector<Point> &lump)
	{
		for (int i = 0; i < lump.size(); i++)
		{
			Erase(lump[i]);
			for (int dir = 0; dir < 4; dir++)
			{
				Point next = lump[i];
				next.Move(dir);
				if (IsOutside(next) || !field[next.x][next.y].IsOjama()) continue;
				AttackOjama(next);
			}
		}
	}

	//�d�͂ŗ������邩�̂悤��, �ʂ����ɋl�߂�
	//��x�̏����̍�, rain[x]�ɋl�܂��Ă���ʂ�K�v�ȕ������ォ��~�点�邪,
	//  ����Ȃ����͉����~�点�����u
	//�����_���ŋʂ��~�点�ăt�B�[���h�𖄂߂Ă����Ƃ������Ƃ͂Ȃ��̂Œ���!
	void Rain()
	{
		for (int x = 0; x < W; x++)
		{
			int y = H - 2;
			for (int targetY = H - 1; targetY >= 0; targetY--)
			{
				if (!field[x][targetY].IsEmpty()) continue;
				y = min(y, targetY - 1);
				//field[x][targetY]�͋�
				while (y >= 0 && field[x][y].IsEmpty()) y--;
				//(y>=0�Ȃ�)field[x][y]�͋�łȂ�
				if (y >= 0) swap(field[x][targetY], field[x][y]);
				else if (!rain[x].empty())
				{
					field[x][targetY] = Cell(rain[x].front());
					rain[x].pop();
				}
				else break;
			}
		}
	}
};

int main()
{
	//�ł������̐F�t���ʂ���������W���×~�I�ɑI��AI
	int w, h, n, m;
	cin >> w >> h >> n >> m;
	cout << "GreedyAI" << endl;
	while (true)
	{
		State myState = State::Input(w, h, n, m);
		State rivalState = State::Input(w, h, n, m);
		Point ans;
		int maxSize = 0;
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				if (!myState.field[x][y].IsColorful()) continue;
				vector<Point> lump = myState.GetLump(Point(x, y));
				if (maxSize < lump.size())
				{
					maxSize = lump.size();
					ans = Point(x, y);
				}
			}
		}
		cout << ans.x + 1 << " " << ans.y + 1 << endl;
	}
	return 0;
}