/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package gamesmanlightserver;

/**
 *
 * @author Stephen
 */

class DBHeader {
    
}

class DBRow {
    public String val;
    public long remoteness;
}

public abstract class AbstractDatabase {
    abstract DBRow getRow(long hash);
    abstract DBHeader getHeader();
    
}
