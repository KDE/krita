# iterators
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkWriteBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkWriteBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkWriteBytes

./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkReadBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkReadBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkReadBytes

./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkConstReadBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkConstReadBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkConstReadBytes

./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkReadWriteBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkReadWriteBytes
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkReadWriteBytes

./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkNoMemCpy
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkNoMemCpy
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkNoMemCpy

./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkConstNoMemCpy
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkConstNoMemCpy
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkConstNoMemCpy

./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkTwoIteratorsNoMemCpy
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkTwoIteratorsNoMemCpy
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkTwoIteratorsNoMemCpy

# own
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkTileByTileWrite
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkTileByTileWrite
./KisRandomIteratorBenchmark -silent -iterations 10 benchmarkTileByTileWrite
# those takes too long
./KisRandomIteratorBenchmark -silent -iterations 1 benchmarkTotalRandom
./KisRandomIteratorBenchmark -silent -iterations 1 benchmarkTotalRandom
./KisRandomIteratorBenchmark -silent -iterations 1 benchmarkTotalRandom

./KisRandomIteratorBenchmark -silent -iterations 1 benchmarkTotalRandomConst
./KisRandomIteratorBenchmark -silent -iterations 1 benchmarkTotalRandomConst
./KisRandomIteratorBenchmark -silent -iterations 1 benchmarkTotalRandomConst




