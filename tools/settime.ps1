Write-Host "Trying to connect to smartwatch on COM6"
$port = new-Object System.IO.Ports.SerialPort COM6,115200,None,8,one
$port.DtrEnable = 1
$port.Handshake = 'XOnXOff'
$port.open()
$port.DiscardInBuffer()
$port.DiscardOutBuffer()
$port.Write(“time ”)
$d = Get-Date -UFormat '%S %M %H %d %u %m %Y'
$port.WriteLine($d)
$port.ReadLine()
$port.DiscardInBuffer()
$port.DiscardOutBuffer()
$port.Close()