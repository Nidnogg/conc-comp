class SharedVar {
  private int nReaders, nWriters;
  private int sharedVar;
  private int reading, writing;
   SharedVar(int nReaders, int nWriters) {
     this.nReaders = nReaders;
     this.nWriters = nWriters;
  }

  public synchronized void startRead(int tid) {
    while(this.writing > 0) {
      try {
          System.out.println("Reader Thread " + tid + " will wait until writing > 0 is false");
          this.wait();
      } catch(InterruptedException e) { System.out.println("Error: " + e.getMessage()); }
    }

    this.reading++;
    System.out.println("Thread " + tid + " will read");
  }

  public synchronized void endRead(int tid) {
    this.reading--;
    System.out.println("Thread " + tid + " stopped reading");
    try {
      System.out.println("Thread " + tid + " will call notifyAll() to free all readers/writers");
      this.notifyAll();
    } catch(IllegalMonitorStateException e) { System.out.println("Error: " + e.getMessage()); }

  }

  public synchronized void startWrite(int tid) {
    while(this.writing > 0 || this.reading > 0) {
      try {
          System.out.println("Writer thread " + tid + " will wait until writing > 0 || reading > 0 is false");
          this.wait();
      } catch(InterruptedException e) { System.out.println("Error: " + e.getMessage()); }
    }

    this.writing++;
    System.out.println("Thread " + tid + " will write");
  }

  public synchronized void endWrite(int tid) {
    this.writing--;
    System.out.println("Thread " + tid + " stopped writing");
    try {
      System.out.println("Thread " + tid + " will call notifyAll() to free all readers/writers");
      this.notifyAll();
    } catch(IllegalMonitorStateException e) { System.out.println("Error: " + e.getMessage()); }
  }

  // Reads
  public int get() {
    return this.sharedVar;
  }

  // Writes
  public void set(int varReplacer) {
    this.sharedVar = varReplacer;
  }

}

class Reader extends Thread {
  int tid, readItem;
  SharedVar s;

  Reader(int tid, SharedVar s) {
    this.tid = tid;
    this.s = s;
  }

  public void run() {
    while(true) {
      this.s.startRead(tid);
      // Leitura
      readItem = s.get();
      System.out.println("Thread " + this.tid + " read " + this.readItem);
      try {
      this.sleep(1000);
      } catch(InterruptedException e) { System.out.println("Error: " + e.getMessage()); }  

      this.s.endRead(tid);
    }
  }

}

class Writer extends Thread {
  int tid;
  SharedVar s;

  Writer(int tid, SharedVar s) {
    this.tid = tid;
    this.s = s;
  }

  public void run() {
    while(true) {
      this.s.startWrite(this.tid);
      // Writes
      s.set(this.tid);
      System.out.println("Thread " + this.tid + " escreveu " + this.tid);
      try {
      this.sleep(1000);
      } catch(InterruptedException e) { System.out.println("Error: " + e.getMessage()); }  

      this.s.endWrite(this.tid);
    }
  }
  
}

class RW {
  public static void main(String[] args) {
    int nReaders = 2;
    int nWriters = 2; 
    Thread[] threads = new Thread[nReaders+nWriters];

    SharedVar s = new SharedVar(nReaders, nWriters);

    for(int i = 0; i < nReaders; i++) {
      threads[i] = new Reader(i, s);
      System.out.println("Created reader thread " + i);
    }

    for(int i = nReaders; i < (nReaders + nWriters); i++) {
      System.out.println("Created writer thread " + i);
      threads[i] = new Writer(i, s);
    }

    for(int i = 0; i < (nReaders + nWriters); i++) {
      System.out.println("Starting thread " + i);
      threads[i].start();
    }

    try {
      for(int i = 0; i < (nReaders + nWriters); i++) {
        threads[i].join();
      }
    } catch(InterruptedException e) { System.out.println("Error: " + e.getMessage()); }

   }
}