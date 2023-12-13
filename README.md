# Cell Simulation, Again

This is probably my third attempt at creating cells.
My goal with this project is to create some basic cellular life that can evolve.
These cells will function as follows:
- Cells have four genes, at least one has to be input, and one output
- Cells can die
- Cells can create other cells, but only if they have enough energy
- Cells cannot be created from nothing
  - Except at the begining
  - Extinction cannot be reverted
- Cells require energy, and will gradually consume it

These rules mentioned three complex things that require their own rules, genetics, death, and energy. \
Genes function as follows:
- Genes can represent an input, or an output
- Genes have a strength, which is applied to the input or to their output
  - Strength can occupy a range of $[-4, 4]$
- Input genes have a range of $[-1, 1]$
- Output genes have a domain of $[-4, 4]$
- Genes are passed on from parents to their children
- Randomly, a single random bit can be flipped in the genes of the child or parent
  - The chance of it happening in the parent or child is $\frac{1}{(2^{16})}$
  - The chance of it happening in both is $\frac{1}{(2^{64})}$
- Cells randomly give birth as long as they have enough energy, with a chance of $\frac{1}{(2^{24})}$ per update.

The rules of death are as follows:
- Death is irreversible
  - Dead things cannot cease to be dead
  - Living things cannot arise from dead things
- Dead things cannot independently act
- Dead things cannot create more dead things
- Living things can die, either by being murdered, running out of energy, suicide, or old age
  - Death due to old age has a random chance of occuring after 1116 updates, with a probibility of $\frac{1}{(2^{32})}$ per update
- The equation of the remaining energy in a cell after it dies is $\text{dead energy} = \begin{cases} 2 & \quad \text{if living energy} < 4 \\ \text{living energy}/2 & \quad \text{if living energy} \le 32 \\  \text{living energy}/4 & \quad \text{otherwise} \end{cases} $

The rules of energy are as follows:
- All cells require energy
- Energy is consumed at $\text{consumption}=1 + \text{actions taken}^{\frac{3}{2}}$
  - This does not apply to murders, which operate at $\text{consumption} = (\text{victim energy} - \text{dead energy}) + 2$
- Energy is always treated as an integer value
- Energy is gained through murder, and consuming dead things
  - Murders grant the $\text{dead energy}$ value of the victim
  - Consuming dead things grants their energy
- Giving birth will consume half of a cell's energy, and create a new cell with a quarter of the cell's energy
