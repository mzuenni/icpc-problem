#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

using namespace std;
using namespace __gnu_pbds;

#define all(x) begin(x), end(x)
#define sz(x) (ll)(x).size()

using ll = long long;
using ld = long double;
using pt = complex<ld>;
template<typename T>
using Tree = tree<T, null_type, less<T>, rb_tree_tag,
				  tree_order_statistics_node_update>;

constexpr ld EPS = 1e-12;

ld cross(pt a, pt b) {return imag(conj(a) * b);}

pt readPT() {
	ld x, y;
	cin >> x >> y;
	return {x, y};
}

struct point {
	pt p;
	ll i;
};

ll inversions(const vector<point>& in) {
	Tree<ll> t; //ordered statistics tree
	ll res = 0;
	for (ll i = 0; i < sz(in); i++) {
		res += i - t.order_of_key(in[i].i);
		t.insert(in[i].i);
	}
	return res;
}

ll solve(const vector<pt>& in, pt e) {
	vector<point> tmp;
	for (ll i = 0; i < sz(in); i++) {
		tmp.push_back({in[i], -1});
	}
	sort(all(tmp), [](const point& a, const point& b){
		if (abs(cross(a.p, b.p)) < EPS) {
			return norm(a.p) > norm(b.p);
		} else {
			return cross(a.p, b.p) < 0;
		}
	});
	for (ll i = 0; i < sz(tmp); i++) {
		tmp[i].i = i;
		tmp[i].p -= e;
	}
	sort(all(tmp), [](const point& a, const point& b){
		if (abs(cross(a.p, b.p)) < EPS) {
			return norm(a.p) < norm(b.p);
		} else {
			return cross(a.p, b.p) < 0;
		}
	});
	return inversions(tmp);
}

int main() {
	ios_base::sync_with_stdio(false);
	cin.tie(nullptr);
	pt s = readPT();
	pt e = readPT();
	e -= s;
	pt orth = e * pt(0, 1);
	ll n;
	cin >> n;
	vector<pt> upper, lower;
	for (ll i = 0; i < n; i++) {
		pt c = readPT();
		c -= s;
		if (cross(e, c) > 0 || (abs(cross(e, c)) < EPS && cross(orth, c) < EPS)) upper.push_back(c);
		else lower.push_back(-c+e);
	}
	cout << solve(upper, e) + solve(lower, e) << endl;
}
