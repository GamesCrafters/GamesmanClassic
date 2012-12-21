/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package gamesmanlightserver;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.Iterator;
import java.util.concurrent.Executors;
import java.util.HashMap;

import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.lang.reflect.Method;
import java.util.ArrayList;

/**
 *
 * @author Stephen
 */
public class GamesmanLightServer {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) throws IOException {
        InetSocketAddress addr = new InetSocketAddress(9999);
        HttpServer server = HttpServer.create(addr, 0);

        server.createContext("/", new APIHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        System.out.println("Server start successful: port " + addr);
    }

    static class APIHandler implements HttpHandler {

        ArrayList<String> acceptableGames = new ArrayList<String>();
        BoardHashUtil util = new BoardHashUtil();
        AbstractDatabase db;
        HashMap<String, Importer> importers;

        public APIHandler() {
            acceptableGames.add("1210");
            //acceptableGames.add("ttt");
            //acceptableGames.add("connect4");
            db = new DemoDatabase();
            importers = new HashMap<String, Importer>();
            try {
                for (int i = 0; i < acceptableGames.size(); i++) {
                    String game = acceptableGames.get(i);
                    Importer newImporter = new Importer(game);
                    importers.put(game, newImporter);
                    System.out.println("Successfully loaded game file: " + game);
                    System.out.println(newImporter.getRow(8));
                }
            } catch (IOException e) {
                System.err.println("A database file was not found");
            }
        }

        private void displayError(HttpExchange exchange, String error) throws IOException {
            Headers responseHeaders = exchange.getResponseHeaders();
            responseHeaders.set("Content-Type", "text/plain");
            exchange.sendResponseHeaders(200, 0);

            OutputStream responseBody = exchange.getResponseBody();
            String s = error;
            responseBody.write(s.getBytes());
            responseBody.close();
        }

        public String getStringFromRow(Row row) {
            String result = "{\"board\":";
            result += row.hash;
            result += ",\"remoteness\":";
            result += row.remoteness;

            result += ",\"value\":\"";
            result += row.result;
            result += "\"}";

            return result;
        }

        public void handle(HttpExchange exchange) throws IOException {
            String requestURI = exchange.getRequestURI().toString();
            String[] args = requestURI.split("/");
            String requestMethod = exchange.getRequestMethod();
            if (requestMethod.equalsIgnoreCase("GET")) {
                if (args.length < 3 || !"api".equals(args[1])) {
                    this.displayError(exchange, "Invalid API request format.");
                    return;
                }

                String puzzle = args[2];

                if (!acceptableGames.contains(puzzle)) {
                    this.displayError(exchange, puzzle + " is not a supported puzzle. Supported: " + acceptableGames);
                    return;
                }

                if (args.length < 4) {
                    this.displayError(exchange, "No game arguments received.");
                    return;
                }

                String[] parameters = args[3].split(";");
                if (parameters.length == 1) {
                    this.displayError(exchange, "Invalid parameters.");
                    return;
                }

                HashMap<String, String> params = new HashMap<String, String>();
                for (int i = 0; i < parameters.length; i++) {
                    String[] splitted = parameters[i].split("=");
                    if (splitted.length < 2) {
                        continue;
                    }
                    params.put(splitted[0], splitted[1]);
                }

                String display = "";

                if (parameters[0].equals("getMoveValue")) {
                    if (!params.containsKey("board")) {
                        this.displayError(exchange, "Required parameter 'board' not found.");
                        return;
                    }

                    long boardPos = Long.parseLong(params.get("board"));
                    if (boardPos < 0 || boardPos > 10) {
                        this.displayError(exchange, "Invalid board position: " + boardPos);
                        return;
                    }

                    //{"status":"ok","response":{"board":"         ","remoteness":9,"value":"tie"}}
                    try {
                        Method toHash = util.getClass().getMethod("toHash_" + puzzle, String.class);
                        long hash = (long) (toHash.invoke(util, params.get("board")));
                        String result = "{\"status\":\"ok\",\"response\":";
                        Importer gameImporter = importers.get(puzzle);
                        Row thisRow = gameImporter.getRow(hash);
                        result += getStringFromRow(thisRow);
                        result += "}";

                        display = result;
                        System.out.println(result);
                    } catch (NoSuchMethodException e) {
                        this.displayError(exchange, "Could not find method toHash_" + puzzle);
                    } catch (Exception e) {
                        this.displayError(exchange, "Exception: " + e);
                    }

                } else if (parameters[0].equals("getNextMoveValues")) {
                    try {
                        Method toHash = util.getClass().getMethod("toHash_" + puzzle, String.class);
                        long hash = (long) (toHash.invoke(util, params.get("board")));
                        Importer gameImporter = importers.get(puzzle);
                        Row thisRow = gameImporter.getRow(hash);
                        long[] successors = thisRow.target_position;
                        String result = "{\"status\":\"ok\",\"response\":[";
                        for (int i = 0; i < successors.length; i++) {
                            Row successorRow = gameImporter.getRow(successors[i]);
                            result += getStringFromRow(successorRow);
                            if (i != successors.length - 1) {
                                result += ",";
                            }
                        }
                        result += "]}";
                        this.displayError(exchange, result);
                    } catch (NoSuchMethodException e) {
                        this.displayError(exchange, "Could not find method toHash_" + puzzle);
                    } catch (Exception e) {
                        this.displayError(exchange, "Exception: " + e);
                    }
                } else {
                    this.displayError(exchange, "Invalid request: '" + parameters[0] + "'. Must be either 'getMoveValue' or 'getNextMoveValues'.");
                    return;
                }

                Headers responseHeaders = exchange.getResponseHeaders();
                responseHeaders.set("Content-Type", "text/plain");
                exchange.sendResponseHeaders(200, 0);

                System.out.println(exchange.getRequestURI());

                OutputStream responseBody = exchange.getResponseBody();
                String s = display;
                responseBody.write(s.getBytes());
                responseBody.close();
            }
        }
    }
}
