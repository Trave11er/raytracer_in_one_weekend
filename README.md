Based on RayTracing in one weekend [book](https://raytracing.github.io/books/RayTracingInOneWeekend.html)

This is a multithreaded version with metaball (lava-lamp) support via ray marching.
It is different from the vanilla version in the following ways:
- Multithreaded rendering with `std::thread`
- Using raw pointers instead of smart ones
- Thread-safe random number generator
- Metaball ray marching with configurable influence radius (`OLD = false` in `src/config.h`)
- Set `OLD = true` to switch back to standard sphere intersection



To build
```
cmake . && make
```

To run (the last number specifies how many threads to use)
```
./inOneWeekend 16
```

Output is written to `image_new.ppm`. For live viewing of the animation:
```
sxiv image_new.ppm
```

TODO TODO insert commit hash after rebase
![myimg](images/final_image.png)


For smoothing viewing of updatable image can use `sxiv`
