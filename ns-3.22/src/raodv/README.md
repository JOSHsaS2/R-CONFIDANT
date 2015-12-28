
```
*Ranked AODV Algorithm*

- Please refer to data_structures/RankBuffer/README for information on the Rank Buffer Object
- Please refer to Steven's implementation of pair<Ipv4Address,*rtable> storage.
- Every node holds a single Rank Buffer that ranks its direct neighbours (1-hop)


*Triggers for using the Rank Buffer in RAODV*

- When a new pair<Ipv4Address,*rtable> object is added, it is also added to the Rank Buffer.

- When a route to a destination node is needed, the Rank Buffer passes on the requests to the (*rtable) pointers in order of Rank, and returns the first result. AODV then searches the resultant (*rtable) as per normal.

- When a node wants to penalise a direct neighbour, it simply calls *deRank()* on the neighbour.
```