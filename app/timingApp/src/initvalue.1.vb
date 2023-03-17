


Type cfgVal
  devName As String
  fileName As String
  hdrY As Integer
  hdrX As Integer  'header point
  offY As Integer
  offX(64) As Integer 'data point offsetX 0~63 from hdrX, value is print or not
  endX As Integer  'data end check point offset from hdrX
  endText(2) As Integer 'endText point from hdrX, second value is print or not
  elsText(2) As Integer 'elsText point from hdrX, second value is print or not
End Type 

Dim i As Integer
Dim j As Integer
Dim retV As Integer


Dim DQM As String       'char["], double quotation marks
Dim SEP As String       'separator



Function genArrayFile(l_devName As String, l_fileName As String, l_hdrY As Integer, l_hdrX As Integer, l_offY As Integer, l_endX As Integer, l_offX0 As Integer, endText As String, elseText As String)
    
    Open l_fileName For Output As #1
    Print #1, "##  " & l_devName & ActiveSheet.Cells(l_hdrY, l_hdrX) & "  ##"
    For i = l_offY To 3000
        If (ActiveSheet.Cells(i, l_endX) = "end") Then
            Print #1, endText & SEP
            Exit For
        End If

        If (ActiveSheet.Cells(i, l_offX0) <> "") Then
            Print #1, ActiveSheet.Cells(i, l_offX0) & SEP
        Else
            Print #1, elseText & SEP
        End If
    Next i
    Close #1

End Function



Function initArray(ByRef aVal() As Integer, size As Integer)
    For i = 0 To size
        aVal(i) = 0
    Next
End Function




Sub fileWrite()
    
    Application.ScreenUpdating = False  '화면 업데이트 (일시) 중지

    DQM = Chr(34)
    SEP = ","
    
    Dim wOrg As Worksheet
    Set wOrg = Worksheets("org")

    Dim sPath As String
    sPath = ThisWorkbook.Path & Application.PathSeparator & ActiveSheet.Cells(3, 1) & ".initValue" & Application.PathSeparator
    'MsgBox strPath
    Dim preFName As String
    Dim postFName As String
    preFName = ActiveSheet.Cells(5, 2)
    Dim fileExt As String
    fileExt = ".init"
    Dim fileName As String

    Dim devName As String
    devName = ActiveSheet.Cells(6, 2)

    DIM cfgV As cfgVal

    If ((ActiveSheet.Cells(5, 1) = "EVG") Or (ActiveSheet.Cells(5, 1) = "EVS")) Then
        
        cfgV.hdrY  = 7
        cfgV.hdrX  = 4
        cfgV.offY  = cfgV.hdrY + 1
        cfgV.endX  = cfgV.hdrX
        cfgV.offX0 = cfgV.hdrX
        cfgV.offX1 = cfgV.hdrX + 3

        For i = offY To 3000
            If (ActiveSheet.Cells(i, endX) = "end") Then
                Exit For
            End If
    
            If (ActiveSheet.Cells(i, offX0) <> "") Then
                Print #1, devName & ActiveSheet.Cells(i, offX0) & SEP & ActiveSheet.Cells(i, offX1) & SEP
            End If
        Next i


        postFName = ActiveSheet.Cells(hdrY, hdrX) & fileExt
        fileName = sPath & preFName & postFName
        Open fileName For Output As #1
        Print #1, "##  " & devName & ActiveSheet.Cells(hdrY, hdrX) & "  ##"
        For i = offY To 3000
            If (ActiveSheet.Cells(i, endX) = "end") Then
                Exit For
            End If
    
            If (ActiveSheet.Cells(i, offX0) <> "") Then
                Print #1, devName & ActiveSheet.Cells(i, offX0) & SEP & ActiveSheet.Cells(i, offX1) & SEP
            End If
        Next i
        Close #1
        
        
        hdrY = 7
        hdrX = 11
        offY = hdrY + 1
        endX = hdrX
        offX0 = hdrX
        postFName = ActiveSheet.Cells(hdrY, hdrX) & fileExt
        fileName = sPath & preFName & postFName
        retV = genArrayFile(devName, fileName, hdrY, hdrX, offY, endX, offX0, "32768", "0")
        
        hdrY = 7
        hdrX = 12
        offY = hdrY + 1
        endX = hdrX - 1
        offX0 = hdrX
        postFName = ActiveSheet.Cells(hdrY, hdrX) & fileExt
        fileName = sPath & preFName & postFName
        retV = genArrayFile(devName, fileName, hdrY, hdrX, offY, endX, offX0, "10", "10")




    End If
    

End Sub














Type Student
  No As Integer
  Name As String
  Score As Integer
End Type


Dim hdrY As Integer
Dim hdrX As Integer     'header point
Dim offY As Integer
Dim offX0 As Integer    'data point offset from hdrX
Dim offX1 As Integer    'data point offset from hdrX
Dim offX2 As Integer    'data point offset from hdrX
Dim offX3 As Integer    'data point offset from hdrX
Dim offX4 As Integer    'data point offset from hdrX
Dim offX5 As Integer    'data point offset from hdrX
Dim offX6 As Integer    'data point offset from hdrX
Dim offX7 As Integer    'data point offset from hdrX
Dim endX As Integer     'data end check point offset from hdrX

Dim i As Integer
Dim j As Integer
Dim retV As Integer

'Const ptX_max As Integer = 100
'Dim ptX(ptX_max) As Integer
'Dim ptXend As Integer

Dim DQM As String       'char["], double quotation marks
Dim SEP As String       'separator



Function genArrayFile(l_devName As String, l_fileName As String, l_hdrY As Integer, l_hdrX As Integer, l_offY As Integer, l_endX As Integer, l_offX0 As Integer, endText As String, elseText As String)
    
    Open l_fileName For Output As #1
    Print #1, "##  " & l_devName & ActiveSheet.Cells(l_hdrY, l_hdrX) & "  ##"
    For i = l_offY To 3000
        If (ActiveSheet.Cells(i, l_endX) = "end") Then
            Print #1, endText & SEP
            Exit For
        End If

        If (ActiveSheet.Cells(i, l_offX0) <> "") Then
            Print #1, ActiveSheet.Cells(i, l_offX0) & SEP
        Else
            Print #1, elseText & SEP
        End If
    Next i
    Close #1

End Function








Sub fileWrite()
    
    
    Application.ScreenUpdating = False  '화면 업데이트 (일시) 중지

    DQM = Chr(34)
    SEP = ","
    
    Dim wOrg As Worksheet
    Set wOrg = Worksheets("org")
    
    Dim sPath As String
    sPath = ThisWorkbook.Path & Application.PathSeparator & ActiveSheet.Cells(3, 1) & ".initValue" & Application.PathSeparator
    'MsgBox strPath
    
    Dim preFName As String
    Dim postFName As String
    preFName = ActiveSheet.Cells(5, 2)
    
    Dim devName As String
    devName = ActiveSheet.Cells(6, 2)
    
    Dim fileExt As String
    fileExt = ".init"
    
    Dim fileName As String

    If ((ActiveSheet.Cells(5, 1) = "EVG") Or (ActiveSheet.Cells(5, 1) = "EVS")) Then
        
        hdrY = 7
        hdrX = 4
        offY = hdrY + 1
        endX = hdrX
        offX0 = hdrX
        offX1 = hdrX + 3
        postFName = ActiveSheet.Cells(hdrY, hdrX) & fileExt
        fileName = sPath & preFName & postFName
        Open fileName For Output As #1
        Print #1, "##  " & devName & ActiveSheet.Cells(hdrY, hdrX) & "  ##"
        For i = offY To 3000
            If (ActiveSheet.Cells(i, endX) = "end") Then
                Exit For
            End If
    
            If (ActiveSheet.Cells(i, offX0) <> "") Then
                Print #1, devName & ActiveSheet.Cells(i, offX0) & SEP & ActiveSheet.Cells(i, offX1) & SEP
            End If
        Next i
        Close #1
        
        
        hdrY = 7
        hdrX = 11
        offY = hdrY + 1
        endX = hdrX
        offX0 = hdrX
        postFName = ActiveSheet.Cells(hdrY, hdrX) & fileExt
        fileName = sPath & preFName & postFName
        retV = genArrayFile(devName, fileName, hdrY, hdrX, offY, endX, offX0, "32768", "0")
        
        hdrY = 7
        hdrX = 12
        offY = hdrY + 1
        endX = hdrX - 1
        offX0 = hdrX
        postFName = ActiveSheet.Cells(hdrY, hdrX) & fileExt
        fileName = sPath & preFName & postFName
        retV = genArrayFile(devName, fileName, hdrY, hdrX, offY, endX, offX0, "10", "10")




    End If
    

End Sub
