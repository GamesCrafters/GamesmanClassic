/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package gamesmanlightserver;

/**
 *
 * @author Stephen
 */

class Header {
    
}

class Row {
    public String val;
    public long remoteness;
}

public abstract class AbstractDatabase {
    abstract Row getRow(long hash);
    abstract Header getHeader();
    
}
