// Write P2(A,B) if there is a path of length 1 or 2 from A to B.
// For any given a, there are at most 6 b s.t. P2(A,B)
// Because A only has two children, and those children each only have two children
// So the total number of P2(A,B) is at most 6N
// So there must be some B s.t. there are at most 6 P2(A,B).
// B has at most 6 paths in, and at most 6 paths out. So regardless of the colors
// of those neighbors, there will be a color left for B.
// So we can remove B, recursively color the rest of graph, and then choose whatever
// color is leftover for B.
#include <vector>
#include <deque>
#include <iostream>
#include <cassert>
#include <queue>
using namespace std;
using ll = int64_t;

int main() {
  ll T;
  cin >> T;
  for(ll cas=1; cas<=T; cas++) {
    ll n;
    cin >> n;
    vector<ll> L(n, 0);
    for(ll i=0; i<n; i++) {
      cin >> L[i];
      L[i]--;
    }
    vector<ll> R(n, 0);
    for(ll i=0; i<n; i++) {
      cin >> R[i];
      R[i]--;
    }
    // P2[i] = list of nodes we can reach <= 2 steps from i
    // i.e. children of i and their children
    // this is always exactly 6 (not necessarily distinct) nodes
    vector<vector<ll>> P2(n, vector<ll>{});
    for(ll i=0; i<n; i++) {
      P2[i].push_back(L[i]);
      P2[i].push_back(L[L[i]]);
      P2[i].push_back(R[L[i]]);
      P2[i].push_back(R[i]);
      P2[i].push_back(L[R[i]]);
      P2[i].push_back(R[R[i]]);
    }

    bool ok = true;
    // If there is a cycle i->j->i, it is impossible
    for(ll i=0; i<n; i++) {
      for(ll x : P2[i]) {
        if(x==i) { ok=false; }
      }
    }
    if(!ok) {
      cout << "Case #" << cas << ": IMPOSSIBLE" << endl;
      continue;
    }

    // Figure out the in-degree of every node
    vector<ll> IN(n, 0);
    for(ll i=0; i<n; i++) {
      for(ll x : P2[i]) {
        IN[x]++;
      }
    }
    deque<ll> O;
    queue<ll> Q;
    // Figure out the order we'll color the nodes in
    // Once we find an order, we can just color greedily
    // (i.e. use any color that hasn't been ruled-out yet).
    // We'll color the "easiest" nodes (ones with low-indegree) last.
    // Specifically, any node with in-degree <= 6 can be colored at the very end;
    // even if every other node is committed to a color, we can still color this one.
    for(ll i=0; i<n; i++) {
      if(IN[i] <= 6) {
        Q.push(i);
      }
    }
    vector<ll> SEEN(n, false);
    while(!Q.empty()) {
      ll x = Q.front(); Q.pop();
      if(SEEN[x]) { continue; }
      SEEN[x] = true;
      assert(IN[x] <= 6);
      O.push_front(x);
      for(ll y : P2[x]) {
        // We're coloring x *after* y, so we don't need to take x into account when
        // figuring out how hard it is to color y. If y has in-degree 6 *not counting
        // later nodes*, we can color y now.
        IN[y]--;
        if(IN[y]<=6) {
          Q.push(y);
        }
      }
    }

    // Now actually do the greedy coloring. BAD[i][c] means that we aren't allowed to use
    // color c for room i. We've chosen an order so that we'll always have at least one
    // available color for each room as it comes up.
    vector<vector<ll>> BAD(n, vector<ll>(13, 0));
    vector<ll> COLOR(n, -1);
    for(ll x : O) {
      // Make sure we don't color x the same as any of its children.
      for(ll y : P2[x]) {
        if(COLOR[y] >= 0) {
          BAD[x][COLOR[y]] = true;
        }
      }
      // Find a color for x.
      for(ll c=0; c<13; c++) {
        if(!BAD[x][c]) {
          COLOR[x] = c;
        }
      }
      assert(COLOR[x] >= 0);
      // Make sure none of our children use that color.
      for(ll y : P2[x]) {
        BAD[y][COLOR[x]] = true;
      }
    }

    // Print out the answer.
    string colors = "ACDEHIJKMORST";
    cout << "Case #" << cas << ": ";
    for(ll i=0; i<n; i++) {
      cout << colors[COLOR[i]];
    }
    cout << endl;
  }
}
