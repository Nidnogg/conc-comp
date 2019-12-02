class SharedArray {
  int arr[];

  public SharedArray(int arrSize) {
    this.arr = new int[arrSize];
  }

  public int[] get() {
    return this.arr;
  }

  public void initialize(int value) {
    for(int i = 0; i < this.arr.length; i++) {
      this.arr[i] = value;
    }

  }

  public synchronized void sumElemArray(int i, int elemA, int elemB) {
    this.arr[i] = elemA + elemB;
  }
}

class IncrementerThread extends Thread {
  int tid, nThreads;
  SharedArray arrA, arrB, arrC;
  
  IncrementerThread(int tid, int nThreads, SharedArray arrA, SharedArray arrB, SharedArray arrC) {
    this.tid = tid;
    this.nThreads = nThreads;
    this.arrA = arrA;
    this.arrB = arrB;
    this.arrC = arrC;
  }
  
  // Function runs when thread is called
  public void run() {
    for(int i = tid; i < arrC.get().length; i += nThreads) {
      System.out.println("Thread " + tid);
      this.arrC.sumElemArray(i, this.arrA.get()[i], this.arrB.get()[i]);
      System.out.println("Setting arrC[" + i + "] to arrA[" + i + "] (" + arrA.get()[i] + ") + arrB[" + i + "] (" + arrB.get()[i] + ")" );
    }
  }

}

class ArraySum {
  final static int arrSize = 5;
  final static int nThreads = 2;

  public static void main(String[] args) {

    Thread[] threads = new Thread[nThreads];

    SharedArray arrA = new SharedArray(arrSize);
    SharedArray arrB = new SharedArray(arrSize);
    SharedArray arrC = new SharedArray(arrSize);
    
    arrA.initialize(1);
    arrB.initialize(1);
    arrC.initialize(0);

    for (int i = 0; i < nThreads; i++) {
      threads[i] = new IncrementerThread(i, nThreads, arrA, arrB, arrC);
      threads[i].run();
    }

    for (int i = 0; i < nThreads; i++) {
      try { threads[i].join(); } catch (InterruptedException e) { }
    }
    
    System.out.println("C array after sum:");
    for (int i = 0; i < arrSize; i++) {
      System.out.print(arrC.get()[i] + " ");
    }
    System.out.print("\n");

  }
}