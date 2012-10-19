// Othello.cpp : Defines the entry point for the console application.
//

#include<iostream>
#include<sstream>
#include<ctime>

#include<string>

using namespace std;

class Board {
	int squares[8][8];

public:
	Board();
	string toString();
	bool play_square(int, int, int);
	bool move_is_valid(int, int, int);
	bool check_or_flip_path(int, int, int, int, int, bool);
	int get_square(int, int);
	int score();
	bool full_board();
	bool has_valid_move(int);
	void set_squares(Board *b); //copy over another board's squares
	int eval(int, int); //heuristic evaluation of a current board for use in mimimax
	int free_neighbors(int, int);
};

pair<int, int> minimax_decision(Board *b, int cpuval);
int max_value(Board *b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start);
int min_value(Board *b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start);

Board::Board() {
	for(int i=0; i<8;i++)
		for(int j=0; j<8; j++)
			squares[i][j] = 0;
	squares[3][3]=-1;
	squares[4][4]=-1;
	squares[3][4]=1;
	squares[4][3]=1;
}

string Board::toString() {
	stringstream s;
	char cforvalplusone[] = {'W', '_', 'B'};
	s << "  1 2 3 4 5 6 7 8" << endl;
	for(int i=0; i<8;i++) {
		s << i+1 << '|';
		for(int j=0; j<8; j++)
			s << cforvalplusone[squares[i][j]+1] << '|';
		s << endl;
	}
	return s.str();
}

//returns if player with val has some valid move in this configuration
bool Board::has_valid_move(int val) {
	for(int i=0; i<8;i++)
		for(int j=0; j<8; j++)
			if(move_is_valid(i+1, j+1, val))
				return true;
	return false;
}

//r and c zero indexed here
//checks whether path in direction rinc, cinc results in flips for val
//will actually flip the pieces along path when doFlips is true
bool Board::check_or_flip_path(int r, int c, int rinc, int cinc, int val, bool doFlips) {
	int pathr = r + rinc;
	int pathc = c + cinc;
	if(pathr < 0 || pathr > 7 || pathc < 0 || pathc > 7 || squares[pathr][pathc]!=-1*val)
		return false;
	//check for some chip of val's along the path:
	while(true) {
		pathr += rinc;
		pathc += cinc;
		if(pathr < 0 || pathr > 7 || pathc < 0 || pathc > 7 || squares[pathr][pathc]==0)
			return false;
		if(squares[pathr][pathc]==val) {
			if(doFlips) {
				pathr=r+rinc;
				pathc=c+cinc;
				while(squares[pathr][pathc]!=val) {
					squares[pathr][pathc]=val;
					pathr += rinc;
					pathc += cinc;
				}
			}
			return true;
		}
	}
	return false;	
}


//returns whether given move is valid in this configuration
bool Board::move_is_valid(int row, int col, int val) {
	int r = row-1;
	int c = col-1;
	if(r < 0 || r > 7 || c < 0 || c > 7)
		return false;
	//check whether space is occupied:
	if(squares[r][c]!=0)
		return false;
	//check that there is at least one path resulting in flips:
	for(int rinc = -1; rinc <= 1; rinc++)
		for(int cinc = -1; cinc <= 1; cinc++) {
			if(check_or_flip_path(r, c, rinc, cinc, val, false))
				return true;
		}
	return false;
}

//executes move if it is valid.  Returns false and does not update board otherwise
bool Board::play_square(int row, int col, int val) {
	if(!move_is_valid(row, col, val))
		return false;
	squares[row-1][col-1] = val;
	for(int rinc = -1; rinc <= 1; rinc++)
		for(int cinc = -1; cinc <= 1; cinc++) {
			check_or_flip_path(row-1, col-1, rinc, cinc, val, true);
		}
	return true;
}

bool Board::full_board() {
	for(int i=0; i<8;i++)
		for(int j=0; j<8; j++)
			if(squares[i][j]==0)
				return false;
	return true;
}

//returns score, positive for X player's advantage
int Board::score() {
	int sum =0;
	for(int i=0; i<8;i++)
		for(int j=0; j<8; j++)
			sum+=squares[i][j];
	return sum;
}

int Board::get_square(int row, int col) {
	return squares[row-1][col-1];
}

void Board::set_squares(Board *b) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			squares[i][j] = b->get_square(i+1,j+1);
		}
	}
}

int Board::eval(int cpuval, int depth) { // originally used score, but it led to bad ai
					// instead we evaluate based maximizing the
					// difference between computer's available move count
					// and the player's. Additionally, corners will be
					// considered as specially beneficial since they cannot ever be
					// flipped.

	int score = 0; // evaluation score

	// count available moves for computer and player
	int mc = 0; int mp = 0;
	for (int i=1; i<9;i++) {
		for (int j=1; j<9; j++) {
			if (move_is_valid(i, j, cpuval))
				mc++;
			if (move_is_valid(i, j, -1*cpuval))
				mp++;
		}
	}

	// add the difference to score (scaled)
	score += 20*(mc - mp); // the number is just some scale determined through playing
	//score += 7*mc;

	/*
	// additionally, if mp is 0 this is real good so add some more points,
	// and if mc is 0 this is real bad so subtract more points
	// because of skipped turns

	if (mp == 0)
		score += 50;
	if (mc == 0)
		score -= 50;
	*/

	// count corners for computer and player
	int cc = 0; int cp = 0;
	if (get_square(1, 1) == cpuval)
		cc++;
	else if (get_square(1, 1) == -1*cpuval)
		cp++;

	if (get_square(1, 8) == cpuval)
		cc++;
	else if (get_square(1, 8) == -1*cpuval)
		cp++;

	if (get_square(8, 1) == cpuval)
		cc++;
	else if (get_square(8, 1) == -1*cpuval)
		cp++;

	if (get_square(8, 8) == cpuval)
		cc++;
	else if (get_square(8, 8) == -1*cpuval)
		cp++;

	// add the difference to score (scaled)
	score += 200*(cc - cp);

	/*
	// squares adjacent to corners on edges also useful, but not as much since it could lead to a corner
	int ac = 0; int ap = 0;
	if (get_square(1, 2) == cpuval)
		ac++;
	else if (get_square(1, 2) == -1*cpuval)
		ap++;
	if (get_square(2, 1) == cpuval)
		ac++;
	else if (get_square(2, 1) == -1*cpuval)
		ap++;

	if (get_square(1, 7) == cpuval)
		ac++;
	else if (get_square(1, 7) == -1*cpuval)
		ap++;
	if (get_square(2, 8) == cpuval)
		ac++;
	else if (get_square(2, 8) == -1*cpuval)
		ap++;

	if (get_square(7, 1) == cpuval)
		ac++;
	else if (get_square(7, 1) == -1*cpuval)
		ap++;
	if (get_square(8, 2) == cpuval)
		ac++;
	else if (get_square(8, 2) == -1*cpuval)
		ap++;

	if (get_square(7, 8) == cpuval)
		ac++;
	else if (get_square(7, 8) == -1*cpuval)
		ap++;
	if (get_square(8, 7) == cpuval)
		ac++;
	else if (get_square(8, 7) == -1*cpuval)
		ap++;

	score += 30*(ac - ap);

	// scale so bigger depths are worth more
	score += 10*depth;

	*/

	// limit the amount of space around our pieces so we don't surround as much (which leads to big gains endgame for opponent)
	int sc = 0; int sp = 0; // counts for open spaces neighboring a player/comp's pieces
	for (int i=1; i<9;i++) {
		for (int j=1; j<9; j++) {
			if (get_square(i, j) == cpuval) {
				//add count to sc
				sc += free_neighbors(i, j);
			}
			if (get_square(i, j) == -1*cpuval) {
				//add count to sp
				sp += free_neighbors(i, j);
			}
		}
	}

	score -= 10*(sc - sp); // subtract because we are trying to minimize it
	return score;
}

int Board::free_neighbors(int i, int j) {
	int count = 0;

	// examine the 8 possible neighborings unless not possible positions
	if ((i+1)>0 && j>0 && (i+1)<9 && j<9 && get_square(i+1, j) == 0)
		count++;
	if ((i+1)>0 && (j-1)>0 && (i+1)<9 && (j-1)<9 && get_square(i+1, j-1) == 0)
		count++;
	if (i>0 && (j-1)>0 && i<9 && (j-1)<9 && get_square(i, j-1) == 0)
		count++;
	if ((i-1)>0 && (j-1)>0 && (i-1)<9 && (j-1)<9 && get_square(i-1, j-1) == 0)
		count++;
	if ((i-1)>0 && j>0 && (i-1)<9 && j<9 && get_square(i-1, j) == 0)
		count++;
	if ((i-1)>0 && (j+1)>0 && (i-1)<9 && (j+1)<9 && get_square(i-1, j+1) == 0)
		count++;
	if (i>0 && (j+1)>0 && i<9 && (j+1)<9 && get_square(i, j+1) == 0)
		count++;
	if ((i+1)>0 && (j+1)>0 && (i+1)<9 && (j+1)<9 && get_square(i+1, j+1) == 0)
		count++;

	return count;

}

bool make_simple_cpu_move(Board * b, int cpuval) {
	for(int i=1; i<9;i++)
		for(int j=1; j<9; j++)
			if(b->get_square(i, j)==0)
				if(b->play_square(i, j, cpuval)) 
					return true;
	cout << "Computer passes." << endl;
	return false;
}

bool make_smarter_cpu_move(Board *b, int cpuval) {
	pair<int,int> temp = minimax_decision(b, cpuval);
	if (b->get_square(temp.first, temp.second) == 0) {
		if (b->play_square(temp.first, temp.second, cpuval))
			return true;
	}
	cout << "Computer passes." <<  endl;
	return false;
}

pair<int, int> minimax_decision(Board *b, int cpuval) {
	// returns a pair<int, int> <i, j> for row, column of best move
	Board *bt = new Board();
	bt->set_squares(b);

	int tempval;

	// computer always trying to maximize eval function
	// need a number higher than eval can ever be to treat as initial max val
	int maxval = 9000;
	int maxi;
	int maxj;

	bool nomove = true;

	int depth = 0; // iterative deepening
	// start clock
	time_t start; time_t now;
	time(&start);

	//int temp;

	while (true) {
		depth++; // increase our depth limit for ID

		for (int i = 1; i < 9; i++) {
			for (int j = 1; j < 9; j++) {
				if (bt->get_square(i, j)==0) {
					if (bt->play_square(i, j, cpuval)) {

						tempval = max_value(bt, cpuval, 9000, -9000, 1, depth, start); // start alpha at 9000, beta at -9000
						if (tempval <= maxval) { // found a new minimum max value
							nomove = false;
							maxi = i;
							maxj = j;
							//temp = depth;
							maxval = tempval;
						}
						bt->set_squares(b); // either way, erase the play and try next one
					}
				}
			}
		}
		time(&now);
		if (difftime(now, start) >= 20)
			break;

		// add a little randomness to throw off other computer who thinks we are playing optimally
		// by cutting off at a ID depth sometimes
		//if ( (rand() % 100) <= 3)
		//	break;
	}
	
	pair<int, int> ret;

	
	if (nomove) {
		nomove = false;
		maxi = 1; // just return something so comp can pass
		maxj = 1;
	}	

	ret.first = maxi;
	ret.second = maxj;

	//printf("%d\t%d\n", depth, temp);
	
	return ret;
}

int max_value(Board *b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start) {
	// scoring and heuristics of current board if terminal
	// 2 ways of being terminal: game is over (usually not the case until endgame)
	//    or reached depth limit

	// additionally, if time limit of 20 sec is reached just return the heuristic

	// if the game is over and computer wins, return maximum number possible (9000)
	// if the game is over and player wins, -9000
	// if tie, 0
	if (b->full_board() || (!(b->has_valid_move(cpuval)) && !(b->has_valid_move(-1*cpuval)))) {
		int score = b->score();
		if(score==0)
			return 0;
		else if((score>0 && (cpuval == 1)) || (score<0 && (cpuval == -1)))
			return 9000;
		else
			return -9000;
	}

	// reached depth limit or time limit, score the board according to heuristic function
	time_t now;
	time(&now);
	if (depth == maxdepth || difftime(now, start) >= 20)
		return b->eval(cpuval, depth);
		
	// maximize the min value of successors
	int minval = beta;
	int tempval;

	Board *bt = new Board();
	bt->set_squares(b);

	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (b->get_square(i, j)==0) {
				if (b->play_square(i, j, -1*cpuval)) { // since this is the player's turn, change the val
				
					tempval = min_value(b, cpuval, alpha, minval, depth+1, maxdepth, start); // new alpha/beta corresponding, our minval will always be >= beta
					if (tempval >= minval) { // found a new maximum min value
						minval = tempval;
					}

					b->set_squares(bt); // either way, erase the play and try next one

					// alpha-beta pruning
					if (minval > alpha) {
						return alpha;
					}
				}
			}
		}
	}
	return minval;
}

int min_value(Board *b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start) {
	// scoring and heuristics of current board if terminal

	// 2 ways of being terminal: game is over  or no more valid moves (usually not the case until endgame)
	//    or reached depth limit

	// additionally if time limit reached (20 sec) just ret heuristic scoring

	// if the game is over and computer wins, return maximum number possible (9000)
	// if the game is over and player wins, -9000
	// if the game is over and nobody wins, score 0
	if (b->full_board() || (!(b->has_valid_move(cpuval)) && !(b->has_valid_move(-1*cpuval)))) {
		int score = b->score();
		if(score==0)
			return 0;
		else if((score>0 && (cpuval == 1)) || (score<0 && (cpuval == -1)))
			return 9000;
		else
			return -9000;
	}

	time_t now;
	time(&now);
	// reached depth limit, score the board according to heuristic function
	if (depth == maxdepth || difftime(now, start) >= 20)
		return b->eval(cpuval, depth);

	// minimize the max value of successors
	int maxval = alpha;
	int tempval;

	Board *bt = new Board();
	bt->set_squares(b);

	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (b->get_square(i, j)==0) {
				if (b->play_square(i, j, cpuval)) {

					tempval = max_value(b, cpuval, maxval, beta, depth+1, maxdepth, start); // our maxval always <= alpha
					if (tempval <= maxval) { // found a new maximum min value
						maxval = tempval;
					}

					b->set_squares(bt); // either way, erase the play and try next one

					if (maxval < beta) {
						return beta;
					}
				}
			}
		}
	}
	return maxval;
}

void play(int cpuval) {
	Board * b = new Board();
	int humanPlayer = -1*cpuval;
	int cpuPlayer = cpuval;

	cout << b->toString();
	int consecutivePasses = 0;

	int row, col;

	if (cpuPlayer == -1) { // cpu plays second
		while(!b->full_board() && consecutivePasses<2) {
			//check if player must pass:
			if(!b->has_valid_move(humanPlayer)) {
				cout << "You must pass." << endl;
				consecutivePasses++;
			}
			else {
				consecutivePasses = 0;
				cout << "Your move row (1-8): ";
				cin >> row;
				cout << "Your move col (1-8): ";
				cin >> col;
				if(!b->play_square(row, col, humanPlayer)) {
					cout << "Illegal move." << endl;
					continue;
				}
			}
			//move for computer:
			if(b->full_board())
				break;
			else {
				cout << b->toString() << "..." << endl;
				//if(make_simple_cpu_move(b, cpuPlayer))
				//	consecutivePasses=0;
				if(make_smarter_cpu_move(b, cpuPlayer))
					consecutivePasses=0;
				else
					consecutivePasses++;
				cout << b->toString();
			}
		}
	}
	else { // cpu plays first
		while(!b->full_board() && consecutivePasses<2) {
			//move for computer:
			if(b->full_board())
				break;
			else {
				cout << "..." << endl;
				//if(make_simple_cpu_move(b, cpuPlayer))
				//	consecutivePasses=0;
				if(make_smarter_cpu_move(b, cpuPlayer))
					consecutivePasses=0;
				else
					consecutivePasses++;
				cout << b->toString();
			}

			//check if player must pass:
			if(!b->has_valid_move(humanPlayer)) {
				cout << "You must pass." << endl;
				consecutivePasses++;
			}
			else {
				consecutivePasses = 0;
				while (true) {
					cout << "Your move row (1-8): ";
					cin >> row;
					cout << "Your move col (1-8): ";
					cin >> col;
					if(!b->play_square(row, col, humanPlayer)) {
						cout << "Illegal move." << endl;
					}
					else
						break;
				}
				cout << b->toString();
			}
		}
	}

	int score = b->score();
	if(score==0)
		cout << "Tie game." << endl;
	else if((score>0 && (cpuval == 1)) || (score<0 && (cpuval == -1)))
		cout << "Computer wins by " << abs(score) << endl;
	else
		cout << "Player wins by " << abs(score) << endl;
	char a;
	cin >> a;
}

int main(int argc, char * argv[])
{
	cout << "Enter Y or y if you would like to go first." << endl;
	char a;
	cin >> a;

	if (a == 'y' || a == 'Y') {
		play(-1); // our cpu's val is -1
	}
	else
		play(1);

	return 0;
}