# Game Play

## Stages

1. Stage 1 is nuture. Grain and Deers interact with each other. Wolfs bread and are affected by weather

2. Stage 2 is survive (wolf attack)
All agents interact with each other.

## Time

Each thread should return when the year hits 2020 (giving us 6 years, or 72 months, of simulation).

## Factors



## Agents



### Grain



### Deers



### Wolfs

#### Compute stage 1
* More wolfs are born only when there are more than 1 wolf in the pack. The chance of wolfs being born increase when the temperature and precip is nice. The "nice" condition is harder than grain, so we can use 150% of grain-nuturing condition as wolf-breading condition.

#### Compute stage 2

* The percentage of chance that the wolf pack attack the deers is propotional to the number of wolfs

* In the event of wolf-attack, the bigger the value of grain height, the less deers are killed; but the more deers are existing, the more of them are killed. Also, the more there are wolfs in the attack, the more grain will be storm down during the chase. If there are no wolf-attack in 2 month, half of the wolfs will die.

* The wolfs only come at the end of the month when the grain has grown and the deers are fed (when both values are updated)

* 2 wolf eat 1 deer. If there are more deers than the required numbers, it's fine, but if there are not enough deers for all the wolf, the wolf population will decrease by (required deers - actual deers)/2

# Control Flow


### Watcher
