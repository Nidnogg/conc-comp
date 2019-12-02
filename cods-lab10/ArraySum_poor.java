class ArrayThread extends Thread {
    int tid;
    int a[];
    int b[];
    int c[];


    public ArrayThread(int tid, int[] a, int[] b, int[] c) {
        this.tid = tid;
        this.a = a;
        this.b = b;
        this.c = c;
    }

    public void run() {
        System.out.println("thread " + tid);
        for (int i = this.tid; i < ArraySum.arrSize; i+=ArraySum.nThreads) {
            c[i] = a[i] + b[i];
        }
    }
}

class ArraySum {

    final static int arrSize = 10;
    final static int nThreads = 4;   

    public static void main (String[] args) {
 
        Thread[] threads = new Thread[nThreads];

        int a[] = new int[arrSize];
        int b[] = new int[arrSize];
        int c[] = new int[arrSize];

        for (int i = 0; i < a.length; i++) {
            a[i] = 1;
            b[i] = 2;
            c[i] = 0;
        }

        for (int i = 0; i < threads.length; i++) { 
            final int tid = i;
            threads[i] = new ArrayThread(tid, a, b, c);
            threads[i].start();
        }

        for (int i = 0; i < threads.length; i++) {
            try {
                threads[i].join();
            } catch(InterruptedException e) {
                System.out.println("ga");   
            }
            
        }
        printArray(c);
    }

    public static void printArray(int[] a) {
        for (int i = 0; i < arrSize; i++) {
            System.out.println("c[" + i + "] = " + a[i]);
        }
    }

}