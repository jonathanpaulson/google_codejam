#include <iostream>
#include <set>
#include <queue>
#include <tuple>
#include <cassert>
#include <sstream>
#include <vector>
#include <map>

// An "impartial" game is one where the set of legal moves doesn't depend on whose turn it is
// And you lose if you have no legal moves.
//
// Our game is an example - the legal moves are to delete a vertex and any of its children.
// And you lose once the whole tree is deleted i.e. you can't pick a vertex to delete.
//
// Sprague-Grundy theorem tells us a complete strategy for any impartial game.
// Any position in any impartial game can be assigned a "nimber".
// If the nimber is 0, the player to move will lose. Otherwise, the player to move will win.
//
// The "nimber" of a position is mex({nimber successor positions}).
// "mex" stands for "minimum excluded"; the mex of a list is the smallest natural number that isn't in the list.
// i.e. 0 if 0 is not in the list; 1 if 1 is in the list but 0 is not, etc.
//
// Last key fact: Suppose we split our tree into multiple trees, by removing a vertex in the middle.
// The nimber of the resulting forest is the XOR of the nimbers of the individual trees.
//
// This is all we need to do to solve the problem. We want to find a starting tree with nimber 0 (since that means the
// first player - the computer - will lose). We can compute the nimber of any tree by trying all possible moves,
// compute the nimber of the resulting positions, and taking their mex.
// If we try a bunch of trees with N vertices, hopefully we'll find one with nimber 0.
//
// To actually play out the game, just try all possible moves and make one where the resulting position has nimber 0.
// Since our original position had nimber 0, we know the opponent must move to a position with nimber >0 (that's what it
// means to have nimber 0, that no possible next position has nimber 0). So whatever position they move to will have
// a next position with nimber 0 (that's what it means to have nimber >0).


// A path can never have nimber 0
// 1-2-3-4-5
// We can always split the path into two equal-sized pieces by deleting the middle vertex (or middle 2)
// If you have two identical pieces, you always lose; whatever you do in one piece your opponent can mirror in the other.

using namespace std;
using ll = int64_t;
//using Tree = ll; // path of length n
using Tree = vector<vector<ll>>;

ostream& operator<<(ostream& o, const vector<ll>& A) {
  o << "{";
  for(ll i=0; i<A.size(); i++) {
    o << A[i];
    if(i+1<A.size()) { o << ","; }
  }
  o << "}";
  return o;
}
ostream& operator<<(ostream& o, const Tree& A) {
  o << "{";
  for(ll i=0; i<A.size(); i++) {
    o << i << " " << A[i] << " ";
  }
  o << "}";
  return o;
}

// 1-2-3-4-5-6-7
// delete 4 -> should be 1-2-3 1-2-3
// 1-2-3 5-6-7. relabel "5-6-7" as "1-2-3"; the two pieces are the same!
// 1-2-3 is the same as 3-2-1, but I treat them as different. I could be smarter about that!
//
// Given the original tree and some vertices that have been deleted, break
// it up into several resulting trees
vector<Tree> remove_vertices(const Tree& t, const vector<ll>& SEEN_CONST) {
  vector<ll> SEEN(SEEN_CONST.begin(), SEEN_CONST.end());
  vector<Tree> res;
  for(ll i=0; i<t.size(); i++) {
    if(!SEEN[i]) {
      Tree u;
      ll v = 0;
      map<ll,ll> NAME;
      queue<ll> Q;
      Q.push(i);
      while(!Q.empty()) {
        ll x = Q.front(); Q.pop();
        assert(!SEEN[x]);
        SEEN[x] = true;
        if(NAME.count(x)==0) { NAME[x] = v; v++; }
        while(u.size() <= NAME[x]) { u.push_back({}); }
        //cerr << "x=" << x << " t.size=" << t.size() << endl;
        assert(0<=x && x<t.size());
        //cerr << " t[x]=" << t[x] << endl;
        for(ll y : t[x]) {
          if(SEEN[y]) { continue; }
          if(NAME.count(y)==0) { NAME[y]=v; v++; }
          while(u.size() <= NAME[x] || u.size()<=NAME[y]) { u.push_back({}); }
          u[NAME[x]].push_back(NAME[y]);
          u[NAME[y]].push_back(NAME[x]);
          Q.push(y);
        }
      }
      res.push_back(u);
      //assert(u.size() < t.size());
    }
  }
  return res;
}

vector<vector<ll>> moves2(Tree t, const vector<ll>& SEEN) {
  vector<vector<ll>> res;
  for(ll i=0; i<t.size(); i++) {
    if(!SEEN[i]) {
      ll ni = t[i].size();
      for(ll a=0; a<(1<<ni); a++) {
        bool bad = false;
        vector<ll> SEEN2(SEEN.begin(), SEEN.end());
        SEEN2[i] = true;
        for(ll j=0; j<ni; j++) {
          if(a&(1<<j)) {
            if(SEEN[t[i][j]]) {
              bad = true;
            } else {
              SEEN2[t[i][j]] = true;
            }
          }
        }
        if(!bad) {
          res.push_back(SEEN2);
        }
      }
    }
  }
  return res;
}

vector<vector<ll>> moves(Tree t) {
  vector<vector<ll>> res;
  for(ll i=0; i<t.size(); i++) {
    ll ni = t[i].size();
    for(ll a=0; a<(1<<ni); a++) {
      vector<ll> remove(t.size(), false);
      remove[i] = true;
      for(ll j=0; j<ni; j++) {
        if(a&(1<<j)) {
          remove[t[i][j]] = true;
        }
      }
      res.push_back(remove);
    }
  }
  return res;
}

map<Tree,ll> G;
ll grundy(const Tree& tree) {
  if(tree.size()==0) { return 0; }
  if(G.count(tree)==1) { return G[tree]; }
  ll mex = 0;
  vector<ll> MEX(100, false);
  vector<vector<ll>> tss = moves(tree);
  for(const vector<ll>& remove : tss) {
    vector<ll> SEEN(remove.begin(), remove.end());
    auto ts = remove_vertices(tree, SEEN);
    ll ts_grundy = 0;
    for(Tree& t : ts) {
      ll gt = grundy(t);
      //cerr << "\t t=" << t << " gt=" << gt << endl;
      ts_grundy ^= gt;
    }
    //cerr << "tree=" << tree << " ts_grundy=" << ts_grundy << endl;
    MEX[ts_grundy] = true;
    while(MEX[mex]) { mex++; }
  }
  G[tree] = mex;
  return mex;
}

// One central node, and then paths of lengths in B coming off of it
Tree mk_tree(const vector<ll>& B) {
  ll n = 1;
  for(ll x : B) {
    n += x;
  }
  Tree t(n, vector<ll>{});
  ll v = 1;
  for(ll sz : B) {
    for(ll i=1; i<=sz; i++) {
      ll vi = v++;
      if(i==1) {
        t[0].push_back(vi);
        t[vi].push_back(0);
      } else {
        t[vi-1].push_back(vi);
        t[vi].push_back(vi-1);
      }
    }
  }
  return t;
}

ll grundy2(const Tree& t, const vector<ll>& SEEN) {
  ll k = 0;
  for(ll i=0; i<SEEN.size(); i++) {
    if(SEEN[i]) {
      k++;
    }
  }
  if(k==0) { return grundy(t); }

  vector<ll> SEEN2(SEEN.begin(), SEEN.end());
  auto forest = remove_vertices(t, SEEN2);
  ll ans = 0;
  for(auto& tree : forest) {
    ans ^= grundy(tree);
  }
  return ans;
}

void find() {
  map<ll,vector<ll>> GOOD;
  /*GOOD[30] = make_tuple(1,1,27);
    GOOD[32] = make_tuple(1,1,3,26);
    GOOD[34] = make_tuple(0,2,5,26);
    GOOD[35] = make_tuple(0,1,2,31);
    GOOD[36] = make_tuple(0,1,1,33);
    GOOD[37] = make_tuple(1,1,2,32);
    GOOD[38] = make_tuple(0,2,2,33);
    GOOD[40] = make_tuple(0,1,1,37);*/
  //ll N = 40;
  for(ll n=30; n<=40; n++) {
    cerr << "n=" << n << endl;
    bool done = false;
    if(GOOD.count(n)==1) continue;
    for(ll a=0; 1+a+a+a+a+a<=n; a++) {
      for(ll b=a; 1+a+b+b+b+b<=n; b++) {
        for(ll c=b; 1+a+b+c+c+c<=n; c++) {
          for(ll d=c; 1+a+b+c+d+d<=n; d++) {
            ll e = n-1-a-b-c-d;
            if(done) { break; }
            vector<ll> B{a,b,c,d,e};
            Tree t = mk_tree(B);
            assert(t.size() == n);
            ll gt = grundy(t);
            cerr << "n=" << n << " B=" << B << " gt=" << gt << endl;
            if(gt==0) {
              GOOD[t.size()] = B;
              cerr << "GOOD[" << t.size() << "] = " << B << ";" << endl;
              done = true;
            }
          }
        }
      }
    }
  }
}

void bad_turn(vector<ll>& SEEN) {
  ll k; cin >> k;
  for(ll i=0; i<k; i++) {
    ll x;
    cin >> x;
    x--;
    SEEN[x] = true;
    cerr << "bad=" << x << endl;
  }
}

void play() {
  map<ll,vector<ll>> GOOD;
  GOOD[30] = {0,0,1,1,27};
  GOOD[31] = {4,6,6,7,7};
  GOOD[32] = {0,1,1,3,26};
  GOOD[33] = {3,3,4,8,14};
  GOOD[34] = {0,0,1,15,17};
  GOOD[35] = {0,0,1,2,31};
  GOOD[36] = {0,0,1,1,33};
  GOOD[37] = {0,1,1,2,32};
  GOOD[38] = {0,0,2,2,33};
  GOOD[39] = {1,1,4,14,18};
  GOOD[40] = {0,0,1,1,37};
  ll T;
  cin >> T;
  for(ll cas=1; cas<=T; cas++) {
    ll n;
    cin >> n;
    Tree t = mk_tree(GOOD[n]);
    for(ll i=0; i<n; i++) {
      for(ll y : t[i]) {
        if(i<y) {
          cout << (i+1) << " " << (y+1) << endl;
        }
      }
    }
    ll m;
    cin >> m;
    for(ll game=1; game<=m; game++) {
      cerr << "game=" << game << endl;
      vector<ll> SEEN(n, false);
      ll turn = 0;
      while(true) {
        turn++;
        ll left = 0;
        for(ll i=0; i<n; i++) {
          if(!SEEN[i]) {
            left++;
          }
        }

        cerr << "START game=" << game << " turn=" << turn << " left=" << left << " SEEN=" << SEEN << " val=" << grundy2(t, SEEN) << endl;
        for(auto& tree : remove_vertices(t, SEEN)) {
          cerr << "part=" << tree << " gt=" << grundy(tree) << endl;
        }
        if(left==0) { break; }

        bad_turn(SEEN);
        cerr << "AFTER HIS MOVE game=" << game << " turn=" << turn << " left=" << left << " SEEN=" << SEEN << " val=" << grundy2(t, SEEN) << endl;

        for(auto& tree : remove_vertices(t, SEEN)) {
          cerr << "part=" << tree << " gt=" << grundy(tree) << endl;
        }

        // take my turn
        assert(grundy2(t, SEEN) != 0);

        for(auto& move : moves2(t, SEEN)) {
          if(grundy2(t, move) == 0) {
            vector<ll> PRINT;
            for(ll i=0; i<n; i++) {
              if(move[i] && !SEEN[i]) {
                PRINT.push_back(i);
              }
            }

            ll k = 0;
            for(ll i=0; i<n; i++) {
              if(move[i] && !SEEN[i]) {
                k++;
              }
            }
            ostringstream o;
            o << PRINT.size() << endl;
            ll first = -1;
            for(ll x : PRINT) {
              bool ok = true;
              for(ll j : PRINT) {
                bool neighbor = false;
                for(ll y : t[x]) {
                  if(y == j) { neighbor=true; }
                }
                if(j==x) { neighbor=true; }
                if(!neighbor) {
                  ok = false;
                }
              }
              if(ok) { first=x; }
            }
            o << (first+1);
            for(ll j : PRINT) {
              if(j!=first) {
                o << " " << (j+1);
              }
            }
            o << endl;
            cerr << o.str();
            cout << o.str();
            SEEN = move;
            break;
          }
        }
        cerr << "AFTER MY game=" << game << " turn=" << turn << " left=" << left << " SEEN=" << SEEN << " val=" << grundy2(t, SEEN) << endl;
      }
    }
  }
}

int main() {
  find();
  //play();
}
