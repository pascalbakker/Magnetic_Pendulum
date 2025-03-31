### Magnetic Pendulum Chaos Theory


![4 magnet chaos theory](https://raw.githubusercontent.com/pascalbakker/Magnetic_Pendulum/refs/heads/master/Examples/4Magnets.png)

### Info

The goal of this project is to display chaos with the mangentic pendulum. [Youtube video explaining the problem.](https://youtu.be/oVNr5wPHuTs?si=SJc5yWk-k2eWuROM)


Each purple square is a magnet. Each color represents which magnet it ended up being attracted. 

[A quick video explaining the magnetic pendulum](https://www.youtube.com/watch?v=Qe5Enm96MFQ&t=7s)

In main.c:

"k_f","k_m","k_g" are the force constants. 

To add magnets update the "magnets" array and the "numOfMagnets" variable.

### How to build

```
mkdir build && cd $_ && cmake .. && cmake --build .
```

### How to execute
```
./MagneticPendulum
```
