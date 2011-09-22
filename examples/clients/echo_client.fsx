// TODO: switch libzmq.dll to use relative paths!!!

// NOTE:  changing the current directory is the easiest way to ensure
//        the (native) libzmq.dll is available for use in the REPL 
System.Environment.CurrentDirectory <- 
   @"C:\working\projects\ThirdParty\fs-zmq\deploy"
#I @"C:\working\projects\ThirdParty\fs-zmq\deploy"
#r "fszmq.dll"
open fszmq
open fszmq.Context
open fszmq.Polling
open fszmq.Socket

let encode (s:string) = System.Text.Encoding.ASCII.GetBytes(s)
let recvAll' = recvAll >> Array.map System.Text.Encoding.ASCII.GetString
let scanfn = System.Console.ReadLine
let sleep = int >> System.Threading.Thread.Sleep

let request = [| "TITC01"B; "echo"B; "2"B; "1024"B |]
let update  = [| "TITC01"B; "echo"B; "3"B          |]

let main () =
  use context = new Context(1)
  use client  = context |> req
  "tcp://nyc-ws091:5555" |> connect client

  printf "Press <return> to send message"
  scanfn() |> ignore
  request |> sendAll client
  
  let ticket = client |> recvAll'
  match ticket with
  | [| _; _; _; "200"; uuid |] ->  

      printfn "Job created with id: %s" uuid
      sleep 2000 // give the broker a chance to arrange things
      [| encode uuid |] |> Array.append update |> sendAll client
      printf "Waiting for result... "
      
      let result = client |> recvAll'
      match result with 
      | [| _; _; _; _; status; _ |]
      | [| _; _; _; _; status;   |] -> printfn "%s" status
      | x                           -> printfn "Unable to process results."
                                       printfn "got: %A" x
  
  | x -> printfn "Unable to create job."
         printfn "got: %A" x

main()
