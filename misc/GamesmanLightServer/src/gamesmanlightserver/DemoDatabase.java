/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package gamesmanlightserver;

/**
 *
 * @author Stephen
 */
public class DemoDatabase extends AbstractDatabase{
    
    public DBHeader getHeader() {
        return null;
    }
    
    public DBRow getRow(long hash) {
        int[] remoteness = {7,6,5,5,4,3,3,2,1,1,0};
        String[] value = {"win","lose","win","win","lose","win","win","lose","win","win","lose"};
        DBRow row = new DBRow();
        row.val = value[(int)hash];
        row.remoteness = remoteness[(int)hash];
        return row;
    }
}
