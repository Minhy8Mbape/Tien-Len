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
    5 four of kind
    6 double straight
    */

    Move(vector<int> idx = {},
         int t = 0,
         int v = 0,
         int l = 0) {

        indexes = idx;
        type = t;
        value = v;
        len = l;
    }

    vector<int> getIndexes() const {
        return indexes;
    }

    int getType() const {
        return type;
    }

    int getValue() const {
        return value;
    }

    int getLen() const {
        return len;
    }

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

        sort(hand.begin(), hand.end(),
        [](Card a, Card b){

            if(a.getRank() != b.getRank())
                return a.getRank() < b.getRank();

            return a.getSuit() < b.getSuit();
        });
    }

    void showHand() {

        cout << name << ": ";

        for(Card c : hand)
            cout << c.toString() << " ";

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

public:

    /*
    style:
    1 normal
    2 pair/triple priority
    3 single priority
    4 random
    */

    BotPlayer(string n, int st)
        : Player(n) {

        style = st;
    }

    bool isPartOfPair(int idx) {

        int r = hand[idx].getRank();

        int cnt = 0;

        for(Card c : hand) {

            if(c.getRank() == r)
                cnt++;
        }

        return cnt >= 2;
    }

    bool isPartOfStraight(int idx) {

        int r = hand[idx].getRank();

        bool left = false;
        bool right = false;

        for(Card c : hand) {

            if(c.getRank() == r-1)
                left = true;

            if(c.getRank() == r+1)
                right = true;
        }

        return left || right;
    }

    bool canBeat(Move a, Move cur) {

        if(cur.getType() == 0)
            return true;

        // ===== CHAT 2 =====

        // heo don
        if(cur.getType() == 1 &&
           cur.getValue() == 15) {

            if(a.getType() == 5)
                return true;

            if(a.getType() == 6 &&
               a.getLen() >= 3)
                return true;
        }

        // doi heo
        if(cur.getType() == 2 &&
           cur.getValue() == 15) {

            if(a.getType() == 5)
                return true;

            if(a.getType() == 6 &&
               a.getLen() >= 4)
                return true;
        }

        // ===== SAME TYPE =====

        if(a.getType() != cur.getType())
            return false;

        if(a.getType() == 4 &&
           a.getLen() != cur.getLen())
            return false;

        if(a.getType() == 6 &&
           a.getLen() != cur.getLen())
            return false;

        return a.getValue() >
               cur.getValue();
    }

    vector<Move> generateMoves() {

        vector<Move> res;

        int n = hand.size();

        // ===== SINGLE =====
        for(int i=0;i<n;i++) {

            res.push_back(
                Move({i},
                1,
                hand[i].getRank(),
                1)
            );
        }

        // ===== PAIR / TRIPLE =====
        for(int i=0;i<n;i++) {

            for(int j=i+1;j<n;j++) {

                if(hand[i].getRank() ==
                   hand[j].getRank()) {

                    res.push_back(
                        Move({i,j},
                        2,
                        hand[i].getRank(),
                        2)
                    );

                    for(int k=j+1;k<n;k++) {

                        if(hand[k].getRank() ==
                           hand[i].getRank()) {

                            res.push_back(
                                Move({i,j,k},
                                3,
                                hand[i].getRank(),
                                3)
                            );
                        }
                    }
                }
            }
        }

        // ===== TU QUY =====
        for(int i=0;i<n;i++) {

            vector<int> ids;

            ids.push_back(i);

            for(int j=i+1;j<n;j++) {

                if(hand[j].getRank() ==
                   hand[i].getRank()) {

                    ids.push_back(j);
                }
            }

            if(ids.size() == 4) {

                res.push_back(
                    Move(ids,
                    5,
                    hand[i].getRank(),
                    4)
                );
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
                        Move(ids,
                        4,
                        uni[r].first,
                        ids.size())
                    );
                }
            }

            i = j;
        }

        // ===== DOI THONG =====

        vector<pair<int, vector<int>>> pairs;

        for(int i=0;i<n;i++) {

            for(int j=i+1;j<n;j++) {

                if(hand[i].getRank() ==
                   hand[j].getRank()) {

                    pairs.push_back({
                        hand[i].getRank(),
                        {i,j}
                    });
                }
            }
        }

        sort(pairs.begin(), pairs.end());

        int m2 = pairs.size();

        for(int i=0;i<m2;i++) {

            vector<int> ids =
                pairs[i].second;

            int last =
                pairs[i].first;

            for(int j=i+1;j<m2;j++) {

                if(pairs[j].first ==
                   last + 1) {

                    last =
                        pairs[j].first;

                    ids.push_back(
                        pairs[j].second[0]);

                    ids.push_back(
                        pairs[j].second[1]);

                    int len = ids.size()/2;

                    if(len >= 3) {

                        res.push_back(
                            Move(ids,
                            6,
                            last,
                            len)
                        );
                    }
                }
                else break;
            }
        }

        return res;
    }

    int scoreMove(Move mv,
                  bool freeRound) {

        // ===== RANDOM BOT =====
        if(style == 4)
            return rand() % 1000;

        int score = 0;

        // ===== DOI/BA BOT =====
        if(style == 2) {

            if(mv.getType() == 2)
                score += 500;

            else if(mv.getType() == 3)
                score += 450;

            else if(mv.getType() == 6)
                score += 350;

            else
                score += 30;
        }

        // ===== LE BOT =====
        else if(style == 3) {

            if(mv.getType() == 1)
                score += 500;

            else
                score += 50;
        }

        // ===== SMART AI =====

        if(mv.getType() == 1) {

            int v = mv.getValue();

            score += 20;

            score -= v * 6;

            if(v == 15)
                score -= 300;

            int idx =
                mv.getIndexes()[0];

            if(isPartOfPair(idx))
                score -= 80;

            if(isPartOfStraight(idx))
                score -= 70;
        }

        else if(mv.getType() == 2) {

            score += 140;

            score -= mv.getValue()*2;
        }

        else if(mv.getType() == 3) {

            score += 180;
        }

        else if(mv.getType() == 4) {

            int len = mv.getLen();

            score += len * 120;

            if(len >= 5)
                score += 300;
        }

        else if(mv.getType() == 5) {

            score -= 100;

            if(cardCount() <= 6)
                score += 250;
        }

        else if(mv.getType() == 6) {

            score += 200;

            score -= 80;
        }

        // ===== FREE ROUND =====

        if(freeRound) {

            if(mv.getType() >= 2)
                score += 120;
        }

        // ===== END GAME =====

        if(cardCount() <= 5) {

            score += mv.getLen() * 100;
        }

        return score;
    }

    Move chooseMove(const Move& current) override {

        vector<Move> all =
            generateMoves();

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

            int sc =
                scoreMove(mv,
                freeRound);

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

                cards.push_back(
                    Card(r,s)
                );
            }
        }
    }

    void shuffleDeck() {

        random_shuffle(
            cards.begin(),
            cards.end()
        );
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

        for(int i=0;
            i<(int)players.size();
            i++) {

            vector<Card>& h =
                players[i]->getHand();

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

            players[i%4]->addCard(
                deck.draw(i)
            );
        }

        for(auto p : players)
            p->sortHand();

        turn =
            findThreeSpadesPlayer();
    }

    void showAllHands() {

        cout << "\n====================\n";

        for(auto p : players)
            p->showHand();

        cout << "====================\n";
    }

    void play() {

        init();

        dealCards();

        cout << "\n===== GAME START =====\n";

        showAllHands();

        cout << "\n"
             << players[turn]->getName()
             << " co 3♠ va di truoc!\n";

        while(true) {

            Player* p =
                players[turn];

            cout << "\n---------------------\n";

            cout << p->getName()
                 << " TURN\n";

            Move mv =
                p->chooseMove(current);

            if(mv.isPass()) {

                cout << p->getName()
                     << " PASS\n";

                passCount++;
            }
            else {

                cout << p->getName()
                     << " PLAY: ";

                for(int id :
                    mv.getIndexes()) {

                    cout <<
                    p->getHand()[id]
                    .toString()
                    << " ";
                }

                cout << "\n";

                p->removeCards(
                    mv.getIndexes()
                );

                current = mv;

                lastPlayer = turn;

                passCount = 0;
            }

            cout << "Remaining: "
                 << p->cardCount()
                 << " cards\n";

            if(p->cardCount() == 0) {

                cout << "\n====================\n";

                cout << p->getName()
                     << " WIN !!!\n";

                cout << "====================\n";

                break;
            }

            // everybody pass
            if(passCount == 3) {

                cout
                << "\n=== NEW ROUND ===\n";

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
