# lofi library
lofi 1.0 is experimental and contains support for a fiber based job system with lock free data structures such as a lock free pool which utilizes atomic CAS operations.
There is a lock free queue based with a custom memory reclamation pattern. It sacrifices a bit of cache coherency in task switching sections of execution for cache coherency within the tasks, as it is assumed more time will be spent in a particular job than will be spent switching between them anyways.
There is also a database template which can be used to create tables that can be accessed within the jobs.
Each job can receive a pointer (usually to an object to mutate like a database or database table) and a start and end index for iterating over ranges within the job (so operations can be done on many
rows of the tables, ideally some amount that can fit in the cache of a single processor).
Otherwise, it's very similar to the engine foundation presented in the 2016 gdc talk about the naughty dog engine, but with a bunch more metaprogramming with inspo from Odin Holmes and Andrei Alexandrescu.
