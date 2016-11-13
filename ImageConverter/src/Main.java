import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.*;
import java.awt.*;
import java.util.Scanner;

public class Main
{
    public static void main(String[] args)
    {
        Scanner sc = new Scanner(System.in);
        String result = "";

        try
        {
            System.out.println("Image to convert: ");
            String path = sc.nextLine();
            System.out.print("Converting " + path + " ... ");
            File file = new File(path);
            BufferedImage image = ImageIO.read(file);
            int buffer = 0;
            int inlinevalues = 0;
            String binaryBuffer = "";

            for(int i = 0; i < image.getHeight(); i++)
            {
                for (int u = 0; u < image.getWidth(); u++)
                {
                    int clr = image.getRGB(u, i);
                    int red = (clr & 0x00ff0000) >> 16;
                    int green = (clr & 0x0000ff00) >> 8;
                    int blue = clr & 0x000000ff;

                    if (buffer == 8)
                    {
                        int decimal = Integer.parseInt(binaryBuffer,2);
                        String hexStr = "0x";

                        if((Integer.toString(decimal,16)).length() == 1)
                            hexStr += "0";

                        hexStr += Integer.toString(decimal,16);

                        // System.out.print(hexStr + ", ");
                        result += hexStr + ", ";

                        if(inlinevalues==7)
                        {
                            // System.out.println("");
                            inlinevalues = 0;
                        }
                        else
                            inlinevalues++;

                        binaryBuffer = "";
                        buffer = 0;
                    }

                    if (red < 50 && blue < 50 && green < 50)
                        binaryBuffer += "" + 0;
                    else
                        binaryBuffer += "" + 1;

                    buffer++;
                }
            }

            result = result.substring(0, result.length()-2);
            System.out.println("Done! Save result to file? [Y/N]");
            // Graphics g = image.createGraphics();
            char input = sc.nextLine().toLowerCase().charAt(0);

            if(input == 'y')
            {
                String newfilepath = file.getAbsolutePath();
                String[] pathParts = newfilepath.split("/");
                newfilepath = "/";

                for (int i = 1; i < pathParts.length-1; i++)
                {
                    newfilepath += pathParts[i] + "/";
                }

                newfilepath += file.getName().split("\\.")[0] + "_binary";
                System.out.print("Saving binary image to: " + newfilepath + " ... ");
                File f = new File(newfilepath);
                FileOutputStream fos = new FileOutputStream(f);
                fos.write(("unsigned char test_image[] = {" + result + "};").getBytes());
                System.out.println("Done!");
            }
            else
                System.out.println(result);
        }
        catch (Exception e)
        {
            System.out.println(e.getMessage());
        }
    }
}