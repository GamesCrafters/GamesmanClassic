/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package gamesmanlightserver;

/**
 *
 * @author Stephen
 */
public class BoardHashUtil {
    public String toBoardString_1210(long hash) {
        return hash+"";
    }
    public long toHash_1210(String boardString) {
        return Long.parseLong(boardString);
    }
}
