import java.util.*;

public class Jannik_Scanner {
	static class Aint extends ArrayList<Integer> {} 
	static class Sint extends HashSet<Integer> {} 
    
	static int res = 5;
	static int[] dist;
	static Aint[] adj;
	static Aint vis = new Aint();
	static void dfs(int j, int d) {
		if (dist[j] < Integer.MAX_VALUE)
			res = Math.min(res, dist[j] + d);
		else
			vis.add(j);
		dist[j] = Math.min(dist[j], d);
		if (d + 1 >= res) return;
		for (int k : adj[j]) dfs(k, d+1);
	}

	public static void main(String[] args) throws Exception {
		Scanner in = new Scanner(System.in);
		int n = in.nextInt(), m = in.nextInt();

		// throw away positions
		for (int i = 0; i < n; i++) {
			in.nextInt(); in.nextInt();
		}

		adj = new Aint[n];
		Arrays.setAll(adj, e -> new Aint());

		while (m-- > 0) {
			int a = in.nextInt() - 1, b = in.nextInt() - 1;
			adj[a].add(b);
			adj[b].add(a);
		}

		{ // orientate edges
			Aint q = new Aint();
			int[] deg = new int[n];
			for (int i = 0; i < n; i++)
				if ((deg[i] = adj[i].size()) <= 5)
					q.add(i);
			for (int ii = 0; ii < n; ii++) {
				final int i = q.get(ii);
				adj[i].stream().filter(j -> deg[j] > 0 && --deg[j] == 5).forEach(j -> q.add(j));
				adj[i].removeIf(j -> deg[j] == -1);
				Collections.sort(adj[i]);
				deg[i] = -1;
			}
		}

		dist = new int[n];
		Arrays.fill(dist, Integer.MAX_VALUE);
		for (int i = 0; i < n; i++) {
			dfs(i, 0);
			vis.forEach(j -> dist[j] = Integer.MAX_VALUE);
			vis.clear();
		}

		if (res > 4) {
			Sint[] star2 = new Sint[n];
			Arrays.setAll(star2, e -> new Sint());
			for (int i = 0; i < n; i++)
				for (int j = 0; j < adj[i].size(); j++)
					for (int k = 0; k < j; k++)
						if (!star2[ adj[i].get(j) ].add( adj[i].get(k) )) {
							System.out.println(4);
							return;
						}
		}
		System.out.println(res);
	}
}
