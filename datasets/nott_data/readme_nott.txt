About the instances in nott OSA dataset.
ENTITIES
id, group id, space

"id": an identifer for each entity. 
"group id": group the entity is in. 
"space": the space required for each entity. 


ROOMS
id, floor, space, no of adjacent rooms, adjacent list.

"id": an identifier for each room.
"floor": the floor the room is located. 
"space" is the room space. 
"No of adjacent rooms": The number of rooms adjacent rooms to this room.
"adjacent list": A list of rooms (which are adjacent to this room" of size "no of adjacent rooms".
Although not mentioned in the data file, We also assume that a room is adjacent to itself.


In order to create the nearby list for rooms to be used for nearby constraints, you have to use the floor column. Basically all the rooms in the same floor are near to each other. A room is near to itself as well.



CONSTRAINTS
id, constraint type, hardness, subject, target 
id: an identifer for each constraint

constraint type: type for each constraint. Some constraint types are not used in this dataset. 
UNUSED_CONSTRAINT		-1
ALLOCATION_CONSTRAINT		0
NONALLOCATION_CONSTRAINT	1
ONEOF_CONSTRAINT		2
CAPACITY_CONSTRAINT		3
SAMEROOM_CONSTRAINT		4
NOTSAMEROOM_CONSTRAINT		5
NOTSHARING_CONSTRAINT		6
ADJACENCY_CONSTRAINT		7
NEARBY_CONSTRAINT		8
AWAYFROM_CONSTRAINT		9


hardness: The constraint is a hard constraint (should be satisfied all the time) if this is set as 1. If it is set as 0, it is a soft constraint.


subject and target columns:
for
ALLOCATION_CONSTRAINT		0
NONALLOCATION_CONSTRAINT	1
subject: entity, target: room


for
NOTSHARING_CONSTRAINT		6
subject: entity, target: -1 (not used)

for 
CAPACITY_CONSTRAINT		3
subject: room

for 
SAMEROOM_CONSTRAINT		4
NOTSAMEROOM_CONSTRAINT		5
ADJACENCY_CONSTRAINT		7
NEARBY_CONSTRAINT		8
AWAYFROM_CONSTRAINT		9
subject: entity1, target: entity2