import java.io.*;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by Daniel on 21.08.16.
 */
public class XWDReaderTest
{
    private static List<Byte> imgBytes = new ArrayList<Byte>();

    public static boolean getBit(byte b, int pos)
    {
        return ((b >> pos) & 1) > 0;
    }

    public static void main(String[] arguments)
    {
        try
        {
            FileInputStream fis = new FileInputStream("/Users/Daniel/Desktop/test.xwd");
            DataInputStream in = new DataInputStream(fis);

            try
            {
                while (true)
                {
                    byte b = in.readByte();
                    imgBytes.add(b);
                }
            }
            // catch (EOFException eof) { }
            catch (Exception e)
            {
                System.out.println(e.getMessage());
            }

            Object[] imgByteArray = imgBytes.toArray();
            OutputStream out;
        }
        catch(Exception e)
        {
            System.out.println(e.getMessage());
        }
    }
}
