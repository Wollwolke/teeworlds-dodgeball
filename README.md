# Teeworlds Dodgeball Mod

This is a modification for the retro multiplayer shooter [Teeworlds](https://teeworlds.com).

It was originally developed by [Nilaya](https://www.teeworlds.com/forum/viewtopic.php?id=8652) and updated by [Getkey](https://www.teeworlds.com/forum/viewtopic.php?pid=116597).

## How to play

Nilaya wrote:

>Hearts represent balls and they are often placed in the middle of the map at the start of a game with maybe a few in each team's base. To grab a ball you have to hook it and the object of the game is to hit a player from the opposing team.
>As in normal dodgeball, a point will only be awarded if the ball does not hit any wall or surface. Likewise it is also possible to catch a ball while it is in the air in which case the player who threw the ball is killed and the player who caught it gets a point.
>It's really pretty simply.

## Server settings

Additional Server settings for the mod:

| Name                  | Default | Min | Max  | Description                                                                |
| --------------------- | ------- | --- | ---- | -------------------------------------------------------------------------- |
| sv_db_ball_velocity   | 300     | 10  | 3000 | Velocity of the ball after it got shot                                     |
| sv_db_ball_offset_x   | 0       | 0   | 1000 | X offset of the hookable center of the ball with respect to the lower left |
| sv_db_ball_offset_y   | 0       | 0   | 1000 | Y offset of the hookable center of the ball with respect to the lower left |
| sv_db_ball_radius     | 14      | 0   | 1000 | Hitbox radius of the balls                                                 |
| sv_db_welcome_message | ""      | -   | -    | Message to display when clients join                                       |

## Added Features

- Support for UTF-8 characters in player names
  - the current implementation doesn't look for confusables
- Support for DDNet-like stopper-tiles [(Tile 62)](https://wiki.ddnet.tw/game-tiles/entities/) in the front layer
  - the two included Dodgeball Maps are updated to use this tile

## Compiling

Just follow the official guides on the [Teeworlds Website](https://www.teeworlds.com/?page=docs&wiki=hacking).

>This mod is based on Teeworlds v0.6.4, so for compilation you need bam 4.0.

## Docker Image

To just run the latest version, type

    docker run -d -p 8303:8303/udp --name dodgeball wollwolke/teeworlds-dodgeball

In order to be able to add maps or change the config file, add the `-v` argument to map a directory on your host to the container's `/app/data` directory:

    docker run -d -v /path/on/host/autoexec.cfg:/app/data/autoexec.cfg -v /path/on/host/maps:/app/data/maps ...

Alternatively, you can provide your own data directory and mount it all at once:

    docker run -d -v /path/on/host/:/app/data/ ...
