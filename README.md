# My solutions to IOI 2024 Egypt

By [KCFindstr](https://github.com/KCFindstr)

## Nile

Let's sort by ascending order of $W_i$ first. Then in this sorted list, all pairs of artifacts that are sent together must be consecutive, or with at most 1 artifact (which is sent individually) in between.

1. If the indices of two pairs $(i, i')$ and $(j, j')$ overlap, e.g. $i < j
< i' < j'$, we can regroup them to $(i, j)$ and $(i', j')$. Same for pairs contained in each other.

1. If a pair $(i, i')$ is more than 1 artifact away, we can always pair the artifacts in between to reduce the cost. Then it violates the first condition.

Sort $D$ in all queries in increasing order and maintain the answer on a segment tree. From the above observation, we can just maintain the first and last two artifacts within a segment. This is sufficient for merging adjacent segments.

When the value of $D$ changes, it might become possible for two artifacts to pair up that was not feasible before. For each segment we maintain the next $D$ that will trigger an update within it, and only update recursively when necessary.

Since we only care about artifact pairs that are at most 1 artifact away, there are only $O(N)$ possible pairs that can trigger the updates. Time complexity is $O(N \log N + Q)$.

## Message

### 95 pts

I firstly came up with a 95 pts solution - which is randomized but difficult to hack. Assign a magic number to each bit, and use a random number generator with the previous inputs as the seed to generate the XOR mask of the next packet. Then it is almost impossible for the grader to act adversarially. We select a bit position as the "metadata channel", which will send the array $C$ and then the magic number of that channel. With 67 messages, we have $67\times 16 - 1024 - 1 - 31 + 4 = 20$ redundant bits - which is enough to increase the success rate to $(1-2^{-20})^{30}$. Note the extra 4 comes from the bit we select - 16 choices can encode 4 bits.
In addition, B will validate if the sent array $C$ contains exactly 15 `1`s, which further reduces failure.

### 100 pts

The standard solution is very elegant, and is far different from the above approach.

Since A controls 16 bit, A can encode a loop of length 16 with them: The first few packets of the 16 good bits will encode the distance to the next good bit. If distance is $x$, then it will send $(x-1)$ `0`s and one `1` for the first few rounds. It is impossible for C to create a loop of length 16 since they only control 15 bits. When B receives the message, they can find the loop and decode the message.

To encode the loop, we need 31 bits. Message is 1024 bits long. We need a end bit (always `1`) so B knows when to stop. In total, we use exactly $31+1024+1=1056=66\times16$ bits.

## Tree

Code is ready

> TODO: explain

## Hieroglyphs

Code is ready $O(n)$

> TODO: explain

## Mosaic

Let's investigate how the grid is generated from the initial row and column.

- If we ever generated a 1, then its right and bottom neighbors must be 0; And thus its bottom-right cell must be 1, which creates a diagonal of 1s.
- In the other direction, if we have the following pattern:
  ```
  0 0
  0 1
  ```
  Then the row above and the column to the left of this pattern must be the first row and column of the grid. Otherwise, we will have a contradiction no matter how we fill them.

In other words, after the first 2 rows and 2 columns, the grid is just many diagonal lines of 1s. We can brute force the first 3 rows and 3 columns, and then store the ($x-y$) value of each diagonal line. For each query, we divide the subrectangle in the query into bottom left triangle, upper right triangle, and a strip in the middle. Number of `1`s in each part is easy to calculate, just binary search the diagonal line locations and calculate via a prefix sum array.

## Sphinx

The hint in the scoring suggests that we should find all monochromatic connected components first, without worrying about their colors.

### Find components

Let's maintain a list of largest monochromatic connected components that we discovered - that is, if two vertices are in different components of the list, they are either:

- Not connected by vertices we already considered
- Or, have different colors

Initially, the list is empty. We iterate through all vertices, and for each vertex, we try to merge it into some existing components, or put it in its own new component.

To check if the vertex $v$ can be merged into a subset of components, we can:

- Keep the color of $v$ unchanged.
- Keep the colors of vertices in the subset of components unchanged.
- For vertices not in the subset of components that we care about, set their colors to the Sphinx's color.
- Query the number of monochromatic connected components.

We can calculate the expected number of monochromatic connected components if $v$ cannot be merged into any component inside the subset. Note that if $v$ can be merged, then the expected number of components will decrease, so we can distinguish the two cases.

After knowing $v$ can be merged into some components, we can binary search the components recursively to find the exact components that $v$ can be merged into.

### Find colors

Now let's construct a new graph where each connected component is a vertex. In this new graph, we know for sure that any pair of adjacent vertices have different colors.

For each color $c$, we want to check if any vertex in the new graph has this color. We can divide the vertices into 2 groups according to the parity of their distance to vertex $0$. In this way, every vertex in one group is connected to at least one vertex in the other group.

We can set the colors of one group to $c$, and keep the other group unchanged. The query result tells us whether the other group has any vertex with color $c$. We can then binary search again, similar to the previous step.

The other group is symmetric so we can just repeat the process.

Honestly I don't know how to calculate the number of queries required by this algorithm. I have added some pruning techniques otherwise it will exceed the limit. I think in some test cases it used almost all queries, but eventually it succeeded within 2750 queries. Maybe need to do a more detailed analysis.
