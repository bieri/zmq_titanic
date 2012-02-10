// TODO: switch libzmq.dll to use relative paths!!!

// NOTE:  changing the current directory is the easiest way to ensure
//        the (native) libzmq.dll is available for use in the REPL 
System.Environment.CurrentDirectory <- 
   @"C:\working\projects\ThirdParty\fszmq\deploy"
#I @"C:\working\projects\ThirdParty\fszmq\deploy"
#r "fszmq.dll"
open fszmq
open fszmq.Context
open fszmq.Polling
open fszmq.Socket

type date = System.DateTime

let encode (s:string) = System.Text.Encoding.ASCII.GetBytes(s)
let recvAll' = recvAll >> Array.map System.Text.Encoding.ASCII.GetString
let scanfn = System.Console.ReadLine
let sleep = int >> System.Threading.Thread.Sleep

let main () =
  use context = new Context(1)
  use client  = context |> sub
  subscribe client [""B]
  connect client "tcp://nyc-ws093:5558"

  let rec loop() =
    let msg = recvAll' client
    printfn "========================================"
    printfn "[ %A ]" date.Now
    printfn "========================================"
    msg |> Array.iter (printfn "%s")
    loop()
  loop()

main()
