#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string>

using namespace std;

class Card {
private:
    int rank; // 3 -> 15 (15 = 2)
    int suit;

public:
    Card(int r = 3, int s = 0) {
        rank = r;
        suit = s;
    }

    int getRank() const { return rank; }
    int getSuit() const { return suit; }
    
    void setRank(int r) { rank = r; }
    void setSuit(int s) { suit = s; }

    string toString() const {
        string rs;

        if(rank <= 10)
            rs = to_string(rank);
        else if(rank == 11)
            rs = "J";
        else if(rank == 12)
            rs = "Q";
        else if(rank == 13)
            rs = "K";
        else if(rank == 14)
            rs = "A";
        else
            rs = "2";

        string ss;

        if(suit == 0) ss = "♠";
        else if(suit == 1) ss = "♣";
        else if(suit == 2) ss = "♦";
        else ss = "♥";

        return rs + ss;
    }
};
class Move {
private:
    vector<int> indexes;
    int type;
    int value;
    int len;
public:
    /*
    type:
    0 pass
    1 single
    2 pair
    3 triple
    4 straight
    */

    Move(vector<int> idx = {}, int t = 0, int v = 0, int l = 0) {
        indexes = idx;
        type = t;
        value = v;
        len = l;
    }
    vector<int> getIndexes() const { return indexes; }
    int getType() const { return type; }
    int getValue() const { return value; }
    int getLen() const { return len; }

    bool isPass() const {
        return type == 0;
    }
};

class Player {
protected:
    string name;
    vector<Card> hand;
public:
    Player(string n = "Player") {
        name = n;
    }

    virtual ~Player() {}

    string getName() const {
        return name;
    }

    void addCard(Card c) {
        hand.push_back(c);
    }

    void clearHand() {
        hand.clear();
    }

    int cardCount() const {
        return hand.size();
    }

    vector<Card>& getHand() {
        return hand;
    }
    void sortHand() {
        sort(hand.begin(), hand.end(), [](Card a, Card b){
            if(a.getRank() != b.getRank())
                return a.getRank() < b.getRank();
            return a.getSuit() < b.getSuit();
        });
    }

    void showHand() {
        cout << name << ": ";

        for(int i=0;i<(int)hand.size();i++) {
            cout << hand[i].toString() << " ";
        }

        cout << "\n";
    }
    void removeCards(vector<int> idx) {
        vector<bool> del(hand.size(), false);

        for(int x : idx)
            del[x] = true;

        vector<Card> newHand;

        for(int i=0;i<(int)hand.size();i++) {
            if(!del[i])
                newHand.push_back(hand[i]);
        }

        hand = newHand;
    }

    virtual Move chooseMove(const Move& current) = 0;
};
class BotPlayer : public Player {
protected:
    int style;
    /*
    style:
    1 = normal
    2 = pair/triple priority
    3 = single priority
    4 = random
    */

public:
    BotPlayer(string n, int st = 1) : Player(n) {
        style = st;
    }

    bool canBeat(Move a, Move cur) {

        if(cur.getType() == 0)
            return true;

        if(a.getType() != cur.getType())
            return false;

        if(a.getType() == 4 &&
           a.getLen() != cur.getLen())
            return false;

        return a.getValue() > cur.getValue();
    }

    vector<Move> generateMoves() {

        vector<Move> res;

        int n = hand.size();

        // ===== SINGLE =====
        for(int i=0;i<n;i++) {
            res.push_back(
                Move({i},1,hand[i].getRank(),1)
            );
        }

        // ===== PAIR + TRIPLE =====
        for(int i=0;i<n;i++) {

            for(int j=i+1;j<n;j++) {

                if(hand[i].getRank() ==
                   hand[j].getRank()) {

                    res.push_back(
                        Move({i,j},2,
                        hand[i].getRank(),2)
                    );

                    for(int k=j+1;k<n;k++) {

                        if(hand[k].getRank() ==
                           hand[i].getRank()) {

                            res.push_back(
                                Move({i,j,k},3,
                                hand[i].getRank(),3)
                            );
                        }
                    }
                }
            }
        }

        // ===== STRAIGHT =====
        vector<pair<int,int>> uni;

        for(int i=0;i<n;i++) {

            int r = hand[i].getRank();

            if(r <= 14) {

                if(uni.empty() ||
                   uni.back().first != r)
                    uni.push_back({r,i});
            }
        }

        int m = uni.size();

        for(int i=0;i<m;i++) {

            int j = i;

            while(j+1<m &&
                  uni[j+1].first ==
                  uni[j].first + 1)
                j++;

            for(int l=i;l<=j;l++) {

                for(int r=l+2;r<=j;r++) {

                    vector<int> ids;

                    for(int t=l;t<=r;t++)
                        ids.push_back(
                            uni[t].second
                        );

                    res.push_back(
                        Move(ids,4,
                        uni[r].first,
                        ids.size())
                    );
                }
            }

            i = j;
        }

        return res;
    }

    int scoreMove(Move mv, bool freeRound) {

        // ===== BOT RANDOM =====
        if(style == 4) {
            return rand() % 1000;
        }

        int score = 0;

        // ===== BOT DOI/BA =====
        if(style == 2) {

            if(mv.getType() == 2)
                score += 500;

            else if(mv.getType() == 3)
                score += 450;

            else if(mv.getType() == 4)
                score += 150;

            else
                score += 20;

            score -= mv.getValue();
        }

        // ===== BOT LE =====
        else if(style == 3) {

            if(mv.getType() == 1)
                score += 500;

            else if(mv.getType() == 4)
                score += 100;

            else
                score += 20;

            // ưu tiên lá nhỏ
            score -= mv.getValue() * 3;
        }

        // ===== BOT NORMAL =====
        else {

            if(mv.getType() == 4) {

                score += mv.getLen() * 100;

                if(mv.getLen() >= 5)
                    score += 300;
            }

            else if(mv.getType() == 2)
                score += 120;

            else if(mv.getType() == 3)
                score += 100;

            else {

                score += 20;
                score -= mv.getValue() * 5;

                if(mv.getValue() == 15)
                    score -= 200;
            }
        }

        if(freeRound)
            score += 50;

        return score;
    }

    Move chooseMove(const Move& current) override {

        vector<Move> all = generateMoves();

        vector<Move> legal;

        for(auto mv : all) {

            if(canBeat(mv,current))
                legal.push_back(mv);
        }

        if(legal.empty())
            return Move();

        int bestScore = -1e9;

        Move best;

        bool freeRound =
            (current.getType() == 0);

        for(auto mv : legal) {

            int sc = scoreMove(mv, freeRound);

            if(sc > bestScore) {

                bestScore = sc;
                best = mv;
            }
        }

        return best;
    }
};
class Deck {
private:
    vector<Card> cards;

public:
    Deck() {
        reset();
    }

    void reset() {
        cards.clear();

        for(int r=3;r<=15;r++) {
            for(int s=0;s<4;s++) {
                cards.push_back(Card(r,s));
            }
        }
    }
    void shuffleDeck() {
        random_shuffle(cards.begin(), cards.end());
    }

    Card draw(int pos) {
        return cards[pos];
    }
};
class Game {
private:
    vector<Player*> players;

    Move current;

    int turn;
    int passCount;
    int lastPlayer;

public:
    Game() {
        turn = 0;
        passCount = 0;
        lastPlayer = -1;
    }

    void init() {

    players.push_back(
        new BotPlayer("Bot1",1));

    players.push_back(
        new BotPlayer("Bot2",2));

    players.push_back(
        new BotPlayer("Bot3",3));

    players.push_back(
        new BotPlayer("Bot4",4));
}
    int findThreeSpadesPlayer() {

    for(int i=0;i<(int)players.size();i++) {

        vector<Card>& h = players[i]->getHand();

        for(Card c : h) {

            if(c.getRank() == 3 &&
               c.getSuit() == 0) {

                return i;
            }
        }
    }

    return 0;
}
void dealCards() {

    Deck deck;

    deck.shuffleDeck();

    for(auto p : players)
        p->clearHand();

    for(int i=0;i<52;i++) {

        players[i % 4]->addCard(
            deck.draw(i)
        );
    }

    for(auto p : players)
        p->sortHand();

    // người có 3 bích đi trước
    turn = findThreeSpadesPlayer();
}

    void showAllHands() {
        cout << "\n" << players[turn]->getName() << " co 3♠ va di truoc!\n";
        cout << "\n============================\n";

        for(auto p : players) {
            p->showHand();
        }

        cout << "============================\n";
    }

    void play() {

        init();
        dealCards();
        cout << "\n===== GAME START =====\n";

        showAllHands();

        while(true) {

            Player* p = players[turn];

            cout << "\n--------------------------------\n";
            cout << p->getName() << " TURN\n";

            Move mv = p->chooseMove(current);

            if(mv.isPass()) {

                cout << p->getName() << " PASS\n";

                passCount++;
            }
            else {

                cout << p->getName() << " PLAY: ";

                for(int id : mv.getIndexes()) {
                    cout << p->getHand()[id].toString() << " ";
                }
                
                cout << "\n";

                p->removeCards(mv.getIndexes());

                current = mv;

                lastPlayer = turn;

                passCount = 0;
            }

            cout << "Remaining: "
                 << p->cardCount()
                 << " cards\n";
            if(p->cardCount() == 0) {

                cout << "\n========================\n";
                cout << p->getName() << " WIN !!!\n";
                cout << "========================\n";
                break;
            }

            // everybody pass
            if(passCount == 3) {

                cout << "\n=== NEW ROUND ===\n";

                current = Move();
                passCount = 0;
                turn = lastPlayer;

                continue;
            }
            turn = (turn + 1) % 4;
        }
    }

    ~Game() {

        for(auto p : players)
            delete p;
    }
};
int main() {

    srand(time(0));

    int x;

    cout << "Nhap 1 de bat dau: ";
    cin >> x;

    if(x == 1) {

        Game game;
        game.play();
    }

    return 0;
}