package gamesmanlightserver;

import java.io.*;

public class Importer{
    private Header header;
    private RandomAccessFile file;
    private long rowSize;
    Importer(String fileString) throws IOException{
        file = new RandomAccessFile(fileString, "r");
        header = new Header(file);
        
        rowSize = 1 + header.remoteness_size + header.mex_value_size +
                  header.hash_size * header.move_size; 
                  
    }
    
    public Header getHeader(){
        return header;
    }
    
    public Row getRow(long hash){
        try{
            long position = header.headerSize() + rowSize * hash;
            file.seek(position);
            Row row = new Row(file, header);
            row.hash = hash;
            return row;
        } catch (IOException e){
            System.out.println(e);
            return null;
        }
        
    }

}

class Header{
    public long remoteness_size, mex_value_size, hash_size, initial_position;
    public int move_size;
    private final static int REMOTENESS_BYTE = 8; 
    private final static int MEX_BYTE = 8;
    private final static int HAST_BYTE = 8;
    private final static int MOVE_BYTE = 8;
    private final static int IP_BYTE = 8;
    public Header(RandomAccessFile file) throws IOException{
        file.seek(0);
        remoteness_size = readFile(file, REMOTENESS_BYTE);
        mex_value_size = readFile(file, MEX_BYTE);
        hash_size = readFile(file, HAST_BYTE);
        move_size = (int)readFile(file, MOVE_BYTE);
        initial_position = readFile(file, IP_BYTE);
    }
    public static int headerSize(){
        return REMOTENESS_BYTE + MEX_BYTE + HAST_BYTE + MOVE_BYTE + IP_BYTE;
    }
    private long readFile(RandomAccessFile file, long size) throws IOException{
        long result = 0;
        for (long i = 0; i < size; i++){
            int temp = file.readUnsignedByte();
            result += temp << (8 * i);
        }
        return result;
    }
    public String toString(){
        return "remoteness_size: "+remoteness_size+"\nmex_value_size: "+mex_value_size+"\nhash_size: "+hash_size+"\nmove_counts: " + move_size + "\ninitial_position: " + initial_position;
    }
}

class Row{
    public char result;
    public long remoteness, mex_value;
    public long[] target_position;
    private int move_size;
    public long hash;
    public Row(RandomAccessFile file, Header header) throws IOException{
        result = (char)readFile(file, 1);
        remoteness = readFile(file, header.remoteness_size);
        mex_value = readFile(file, header.mex_value_size);
        move_size = header.move_size;
        target_position = new long[move_size];
        for (int i = 0; i < move_size; i++){
            target_position[i] = readFile(file, header.hash_size);
        }
    }
    private long readFile(RandomAccessFile file, long size) throws IOException{
        long result = 0;
        for (long i = 0; i < size; i++){
            int temp = file.readUnsignedByte();
            result += temp << (8 * i);
        }
        return result;
    }
    public String toString(){
        String s = "\n";
        for (int i = 0; i < move_size; i++){
            s += target_position[i] + "\n";
        }
        return "result: "+result+"\nremoteness: "+remoteness+"\nmex_value: "+mex_value+"\nmoves: " + s;
    }
}
