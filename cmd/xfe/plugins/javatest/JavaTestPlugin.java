import netscape.plugin.Plugin;

class JavaTestPlugin extends Plugin {

    public static int fact(int n) {
	if (n == 1)
	    return 1;
	else
	    return n * fact(n-1);
    }

    int foo;
    int foo_slot;

    public void test_ID() {
    }

    public final void testPluginCalls() {
        try {
            System.out.println("java: foo = "+foo);
            foo = 234;
            System.out.println("java: setting foo to "+foo);

            System.out.print("java: calling plugin stuff");
            char k = doPluginStuff("fwoop", 65535);
            System.out.print(" => "+k);
            System.out.println(k == 'f'
			       ? " ok" : " error");

            System.out.println("java: calling void native1()");
            native1();

            System.out.print("java: calling byte native2((byte)0xff)");
            byte b = native2((byte)0xff);
            System.out.print(" => "+b);
            System.out.println(b == ((byte)0xff + 4)
			       ? " ok" : " error");

            System.out.print("java: calling char native3('a')");
            char c = native3('a');
            System.out.print(" => "+c);
            System.out.println(c == 'b'
			       ? " ok" : " error");

            System.out.print("java: calling short native4((short)-65000)");
            short s = native4((short)-65000);
            System.out.print(" => "+s);
            System.out.println(s == ((short)-65000 + 1)
			       ? " ok" : " error");

            System.out.print("java: calling int native5(5)");
            int i = native5(5);
            System.out.print(" => "+i);
            System.out.println(i == fact(5)
			       ? " ok" : " error");

            System.out.print("java: calling long native6(-989898989898)");
            long l = native6(-989898989898);
            System.out.print(" => "+l);
            System.out.println(l == (-989898989898 + 65536)
			       ? " ok" : " error");

            System.out.print("java: calling float native7(3.1415f)");
            float f = native7(3.1415f);
            System.out.print(" => "+f);
            System.out.println(f == (3.1415f + 1.0)
			       ? " ok" : " error");

            System.out.print("java: calling double native8(1e200)");
            double d = native8(1e200);
            System.out.print(" => "+d);
            System.out.println(d == (1e200 + 1e10)
			       ? " ok" : " error");

	    float[] floats = new float[4];
	    floats[0] = 1.1f;
	    floats[1] = 2.2f;
	    floats[2] = 3.3f;
	    floats[3] = 4.4f;
	    short[] shorts = new short[2];
	    shorts[0] = 1;
	    shorts[1] = 32767;
            System.out.print("java: calling String native9(\"hello\", 1, "
                             +l+", "+floats+")");
            String r = native9("hello", 1, l, floats, shorts);
            System.out.print(" => "+r);
            System.out.println(r.equals("ello")
			       ? " ok" : " error");
	    float j = 1.1f * 2.2f;
	    for (i = 0; i < 4; i++, j *= 2.2f) {
		System.out.print("floats["+i+"] = "+floats[i]);
		System.out.println(floats[i] == j
				   ? " ok" : " error");
	    }

        } catch (Throwable e) {
            System.err.println("!!! Exception: "+e);
            e.printStackTrace(System.err);
        }
    }

    public native char doPluginStuff(String s, int i);

    public native void native1();
    public native byte native2(byte x);
    public native char native3(char x);
    public native short native4(short x);
    public native final int native5(int x);	/* final to test caching */
    public native long native6(long x);
    public native float native7(float x);
    public native double native8(double x);
    public native String native9(String x, int i, long l,
				 float[] floats, short[] shorts);
}
