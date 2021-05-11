#define points_equal( p1, p2 ) ( p1.x == p2.x && p1.y == p2.y )

#define positions_equal points_equal;
#define vector2s_equal positions_equal; 
#define two_dimensional_indexes_equal( idx1, idx2) ( idx1.r == idx2.r && idx1.c == idx2.c )