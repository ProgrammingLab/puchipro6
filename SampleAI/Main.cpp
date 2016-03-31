#include <iostream>
#include <cstdio>
#include <vector>
#include <queue>
using namespace std;

const int HARD_OJAMA = -2;
const int NORMAL_OJAMA = -1;
const int EMPTY = 0;
//色のついた普通の玉は, 1以上M以下の整数で表される

const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0}; //上, 右, 下, 左
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
	//座標を指定した方向に1マス動かす
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
		//入力失敗時の終了処理
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
	int weakness; //おじゃまが弱体化された回数
	int ojamaErasure; //おじゃまが消えた回数
	int colorfulErasure; //色付き玉が消えた回数
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
		//TODO 調整
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
	//座標(x,y)の情報はfield[x][y]に入っている
	//field[y][x]ではないので注意!
	vector<vector<Cell> > field;
	//rain[x]には, 列xに降る予定の玉を表す整数を降る順番に詰める
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
	//指定した座標の玉と完全に同じ種類の玉のうち、繋がっているものの座標を返す(指定した座標も含む)
	//usedは既に調べた座標のところにtrueを入れる
	vector<Point> GetLump(const Point &pos, vector< vector<bool> > used) const
	{
		vector<Point> res;
		if (used[pos.x][pos.y]) return res;
		used[pos.x][pos.y] = true;
		getLump(res, pos, field[pos.x][pos.y].kind, used);
		return res;
	}
	//usedを自動で作る方(使い回せるときは外部で宣言して使い回すほうが高速?)
	vector<Point> GetLump(const Point &pos) const
	{
		vector< vector<bool> > used(W);
		for (int i = 0; i < W; i++) used[i].resize(H, false);
		return GetLump(pos, used);
	}

	//玉の落下する処理をRain()でまとめて行えるようにするため, 消された玉のところは空にしておく
	//なお, 実際のゲームには空のマスは存在しないので注意!
	void Erase(const Point &pos)
	{
		field[pos.x][pos.y].kind = EMPTY;
	}
	void AttackOjama(const Point &pos)
	{
		if (!field[pos.x][pos.y].IsOjama()) return;
		field[pos.x][pos.y].kind++;
	}

	//色付き玉の塊が消えた時に巻き込まれるおじゃまを数える
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
				//おじゃまが消えるなら
				if (cntNext[next.x][next.y] == -field[next.x][next.y].kind)
					res.ojamaErasure++;
				//おじゃまが弱体化されるなら
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

	//重力で落下するかのように, 玉を下に詰める
	//列xの処理の際, rain[x]に詰まっている玉を必要な分だけ上から降らせるが,
	//  足りない分は何も降らせず放置
	//ランダムで玉を降らせてフィールドを埋めてくれるということはないので注意!
	void Rain()
	{
		for (int x = 0; x < W; x++)
		{
			int y = H - 2;
			for (int targetY = H - 1; targetY >= 0; targetY--)
			{
				if (!field[x][targetY].IsEmpty()) continue;
				y = min(y, targetY - 1);
				//field[x][targetY]は空
				while (y >= 0 && field[x][y].IsEmpty()) y--;
				//(y>=0なら)field[x][y]は空でない
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
	//最も多くの色付き玉が消せる座標を貪欲的に選ぶAI
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