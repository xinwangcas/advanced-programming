import sys
import os
import re
import time
import md5
import random  
import thread
from sets import Set
import socket
import traceback
from Tkinter import *  
#record the path
#action: 
#        DELETE=0
#        INSERT=1
#        SUBSTITUTION=2
#        SAMEVALUE=3
class Path:
    def __init__(self, i, j, action):
        self.i=i
        self.j=j
        self.action=action
        
    def get_i_index(self):
        return self.i
    def get_j_index(self):
        return self.j
    def get_action_value(self):
        return self.action

class Edit_Distance:

    def __init__(self,strA,strB,cdel,cins,csub,CHAR_WIDTH):
        self.strA=strA
        self.strB=strB
        self.cdel=cdel
        self.cins=cins
        self.csub=csub
        self.CHAR_WIDTH=CHAR_WIDTH
        self.is_eidted=False

    #comput the distance of two strings
    #cdel: the cost of deleting
    #cins: the cost of inserting
    #csub: the cost of substitution
    def edit_distance(self,strA,strB,cdel,cins,csub):
        m=len(strA)
        n=len(strB)
        self.dist= [[0 for col in range(n+1)] for row in range(m+1)]
        self.backtrack_dist=[[0 for col in range(n+1)] for row in range(m+1)]  #for backtrack matrix 
        #initial the array
        self.dist[0][0]=0
        self.backtrack_dist[0][0]='0'
        i=1
        while i<=n:
            self.dist[0][i]=self.dist[0][i-1]+cins
            self.backtrack_dist[0][i]='-'
            i=i+1
        i=1
        while i<=m:
            self.dist[i][0]=self.dist[i-1][0]+cdel
            self.backtrack_dist[i][0]='|'
            i=i+1
        j=1
        i=1
        while j<=n:
            while i<=m:
                if strA[i-1]==strB[j-1]:  #D[i][j]=min(D[i-1][j-1],D[i-1][j]+1,D[i][j-1]+1);  
                    self.dist[i][j]=min(self.dist[i-1][j-1],self.dist[i-1][j]+cdel,self.dist[i][j-1]+cins)
                    #compute backtrack matrix 
                    if self.dist[i][j] == self.dist[i-1][j-1] :
                        self.backtrack_dist[i][j]='\\'
                    elif self.dist[i][j] == self.dist[i-1][j]+cdel :
                        self.backtrack_dist[i][j]='|'
                    else:
                        self.backtrack_dist[i][j]='-'
                    #print self.dist[i][j]
                else:
                    self.dist[i][j]=min(self.dist[i-1][j-1]+csub,self.dist[i-1][j]+cdel,self.dist[i][j-1]+cins)
                    #compute backtrack matrix 
                    if self.dist[i][j] == self.dist[i-1][j-1]+csub :
                        self.backtrack_dist[i][j]='\\'
                    elif self.dist[i][j] == self.dist[i-1][j]+cdel :
                        self.backtrack_dist[i][j]='|'
                    else:
                        self.backtrack_dist[i][j]='-'                    
                    #print self.dist[i][j]
                i=i+1
            i=1
            j=j+1
        #return self.dist
    #return the fixed width string
    def fiexed_width(self,onestr):
        tmplen=len(str(onestr))
        
        nullstr=""
        while tmplen<=self.CHAR_WIDTH:
            nullstr=nullstr+" "
            tmplen=tmplen+1 
        onestr=nullstr+str(onestr)
#        if len(onestr)>self.CHAR_WIDTH:
#            onestr=onestr[0:self.CHAR_WIDTH-1]
        return onestr       
    def get_matrix_strvalue(self,ary_matrix,strA,strB):
        m=len(strA)
        n=len(strB)
        #print "valuse:"+str(ary_matrix[2][3])    
        #get the first line value    
        tmpstr=""
        tmpstr=tmpstr+self.fiexed_width("")+self.fiexed_width("")
        i=0
        while i<n:
            tmpstr=tmpstr+self.fiexed_width(strB[i])
            i=i+1
        tmpstr=tmpstr+"\n"    
#        print "m="+str(m)
#        print "n="+str(n)
#        print "tmpstr="+str(tmpstr)
        j=0
        i=0
        while j<=m:
            if j==0:  #second line
                tmpstr=tmpstr+self.fiexed_width("")
            else:
                tmpstr=tmpstr+self.fiexed_width(strA[j-1])
                print strA[j-1]
            i=0
            while i<=n:
                tmpstr=tmpstr+self.fiexed_width(str(ary_matrix[j][i]))
                i=i+1
            tmpstr=tmpstr+"\n" 
            j=j+1            
        return tmpstr
    #compute the alignment of two strings 
    def get_alignment_value(self,ary_matrix,strA,strB):
        m=len(strA)
        n=len(strB)
        #action value
        DELETE=0
        INSERT=1
        SUBSTITUTION=2
        SAMEVALUE=3
        
        #declare one array to store the actions
        self.action_ary=[]
         
        i=m
        j=n
        #self.action_ary[0]=Path(0,0,SAMEVALUE)
        print "dist[m][n]:"+str(self.dist[m][n])
        while i!=0 and j!=0:
            tmp1=tmp2=tmp3=999999999  #set big value
            if i-1>=0 and j-1>=0:
                tmp1=self.dist[i-1][j-1]
            if i-1>=0:
                tmp2=self.dist[i-1][j]
            if j-1>=0:
                tmp3=self.dist[i][j-1]
            minv=min(tmp1,tmp2,tmp3)
            curv=self.dist[i][j]
            if minv==tmp1:
                if curv==minv:
                    self.action_ary.append(Path(i,j,SAMEVALUE))
#                    print "same:"+str(i)+" "+str(j)
                elif curv==minv+2:
                    self.action_ary.append(Path(i,j,SUBSTITUTION))
#                    print "sub:"+str(i)+" "+str(j)
                i=i-1
                j=j-1
            elif minv==tmp2:
                self.action_ary.append(Path(i,j,DELETE))
#                print "delete:"+str(i)+" "+str(j)
                i=i-1
            elif minv==tmp3:
                self.action_ary.append(Path(i,j,INSERT))
#                print "insert:"+str(i)+" "+str(j)
                j=j-1
    #compute the alignment string result
        tmpstr=""
        oriStr=""
        destStr=""
        sameStr=""
        tmpstr=tmpstr+self.fiexed_width("")
        i=len(self.action_ary)-1
#        print "len:"+str(i)
        is_append=False
        while i>=0:
            onePath=self.action_ary[i]
#            print "i:"+str(i)
#            print "onePath:"+str(onePath.get_i_index())+" "+str(onePath.get_j_index())+" "+str(onePath.get_action_value())
            if onePath.get_action_value()==DELETE:
#                print "delete"
                if onePath.get_i_index()==len(strA):
                    oriStr=oriStr+self.fiexed_width(strA[onePath.get_i_index()-1])
                else:
                    oriStr=oriStr+self.fiexed_width("-")
                sameStr=sameStr+self.fiexed_width(" ")                
            elif  onePath.get_action_value()==INSERT:
#                print "insert"
                oriStr=oriStr+self.fiexed_width("-")
                #destStr=destStr+self.fiexed_width(strB[onePath.get_j_index()-1])  
                sameStr=sameStr+self.fiexed_width(" ")                              
            elif onePath.get_action_value()==SUBSTITUTION:
#                print "sub"
                oriStr=oriStr+self.fiexed_width(strA[onePath.get_i_index()-1])
                #destStr=destStr+self.fiexed_width(strB[onePath.get_j_index()-1])
                sameStr=sameStr+self.fiexed_width(" ")
            elif onePath.get_action_value()==SAMEVALUE:
#                print "same"
                oriStr=oriStr+self.fiexed_width(str(strA[onePath.get_i_index()-1]))
                #destStr=destStr+self.fiexed_width(str(strB[onePath.get_j_index()-1]))
                sameStr=sameStr+self.fiexed_width("|")    
                
            if onePath.get_j_index()==len(strB) and is_append==False:
                destStr=destStr+self.fiexed_width(strB[onePath.get_j_index()-1])
                is_append=True
            elif is_append==False:
                destStr=destStr+self.fiexed_width(strB[onePath.get_j_index()-1]) 
            else:
                destStr=destStr+self.fiexed_width("-")
                                                      
            i=i-1
#        print "oristr:"+oriStr
#        print "destStr:"+destStr
#        print "same:"+sameStr
        tmpstr=tmpstr+oriStr+"\n"
        tmpstr=tmpstr+self.fiexed_width("")+sameStr+"\n"
        tmpstr=tmpstr+self.fiexed_width("")+destStr+"\n"        
        
        return tmpstr  
                               
        
    #return full edit distance matrix valuse
    def get_full_edit_distance_matrix_result(self):
        if self.is_eidted==False:
            self.edit_distance(self.strA,self.strB,self.cdel,self.cins,self.csub)
            self.is_eidted=True
        return "full edit distance matrix:\n"+self.get_matrix_strvalue(self.dist,self.strA,self.strB)+"\n"
    
    #return full edit distance matrix valuse
    def get_backtrack_matrix_result(self):
        if self.is_eidted==False:
            self.edit_distance(self.strA,self.strB,self.cdel,self.cins,self.csub)
            self.is_eidted=True
        return "backtrack matrix:\n"+self.get_matrix_strvalue(self.backtrack_dist,self.strA,self.strB)+"\n"   
    #return full edit distance matrix valuse
    def get_alignment_result(self):
        if self.is_eidted==False:
            self.edit_distance(self.strA,self.strB,self.cdel,self.cins,self.csub)
            self.is_eidted=True
        return "alignment:\n"+self.get_alignment_value(self.dist,self.strA,self.strB)+"\n"
    def get_edit_distance(self):
        return "\n          edit distance:"+str(self.dist[len(self.strA)][len(self.strB)])+"\n"
#global variable    

def btn_compute(strA,strB,cdel,cins,csub,CheckVar1,CheckVar2,CheckVar3,text):
#    print "strA:"+strA.get()
#    print "strB:"+strB.get()
#    print "cdel:"+str(cdel.get())
#    print "cins:"+str(cins.get())
#    print "csub:"+str(csub.get())
#    print "CheckVar1:"+str(CheckVar1.get())
#    print "CheckVar2:"+str(CheckVar2.get())
#    print "CheckVar3:"+str(CheckVar3.get())
    
    edit_dist=Edit_Distance(strA.get(),strB.get(),cdel.get(),cins.get(),csub.get(),5)
    #matrix_strvalue=edit_dist.get_backtrack_matrix_result()
    if CheckVar1.get()==1:
        full_matrix_strvalue=edit_dist.get_full_edit_distance_matrix_result()
        text.insert(INSERT, full_matrix_strvalue)
    if CheckVar2.get()==1:
        backtrack_matrix_strvalue=edit_dist.get_backtrack_matrix_result()
        text.insert(INSERT, backtrack_matrix_strvalue)
    if CheckVar3.get()==1:
        alignment_matrix_strvalue=edit_dist.get_alignment_result()
        text.insert(INSERT, alignment_matrix_strvalue)    
    edit_distance=edit_dist.get_edit_distance()
    text.insert(INSERT,edit_distance)
    
    
if __name__ == '__main__':

    #ary= [[0 for col in range(5)] for row in range(3)]
#    strA="compare"
#    strB="computer"
#
#
#    m=len(strA)
#    n=len(strB) 
#    edit_dist=Edit_Distance(strA,strB,1,1,2,5)
#    #matrix_strvalue=edit_dist.get_backtrack_matrix_result()
#    matrix_strvalue=edit_dist.get_alignment_result()
#    print len(matrix_strvalue)
    matrix_strvalue=""
    root = Tk() 

#    w = Label(root, text="Hello, world!")   
#    t = Text(root,width=20,height=10)
#    t.insert(1.0,'56789\n')
#    t.insert(3.2,'56789')
#    t.pack()   ,width=m+2,height=(n+2)*5  
    frame= Frame(height = 2000,width = 600,bg = 'red')  
    scrollbar = Scrollbar(frame)
    scrollbar.pack( side = RIGHT, fill=Y )
    text = Text(frame,yscrollcommand = scrollbar.set)
    text.insert(INSERT, matrix_strvalue)
    text.pack(side = LEFT, fill = BOTH )

    scrollbar.config( command = text.yview )    
    frame.pack()
    
    fm = []
    #fram 1
    fm.append(Frame(height = 20,width = 400,bg = 'red'))
    Label(fm[0], text="    first string:").pack(side = LEFT)
    strA=  StringVar()
    Entry(fm[0] ,textvariable =strA).pack(side = RIGHT)    
    #fram 2
    fm.append(Frame(height = 20,width = 400,bg =  'red'))
    Label(fm[1], text="second string:").pack(side = LEFT)
    strB=  StringVar()
    Entry(fm[1] ,textvariable =strB).pack(side = RIGHT)     
    #fram 3
    fm.append(Frame(height = 20,width = 400))
    cdel=IntVar()
    cins=IntVar()
    csub=IntVar()   
    Label(fm[2], text="cdel:").pack(side = LEFT)    
    Scale(fm[2],from_ = 0, to = 50, resolution = 1, orient = HORIZONTAL,variable = cdel).pack(side = RIGHT)
    #fram 4
    fm.append(Frame(height = 20,width = 400))
    Label(fm[3], text="cins:").pack(side = LEFT) 
    Scale(fm[3],from_ = 0, to = 50, resolution = 1, orient = HORIZONTAL,variable = cins).pack(side = RIGHT)
    #fram 5
    fm.append(Frame(height = 20,width = 400))
    Label(fm[4], text="csub:").pack(side = LEFT)     
    Scale(fm[4],from_ = 0, to = 50, resolution = 1, orient = HORIZONTAL,variable = csub).pack(side = RIGHT)
    #fram 6
    fm.append(Frame(height = 20,width = 400))
    CheckVar1 = IntVar()
    CheckVar2 = IntVar()
    CheckVar3 =IntVar()      
    C1 = Checkbutton(fm[5], text = "distance matrix", variable = CheckVar1, \
                     onvalue = 1, offvalue = 0, height=5, \
                     width = 20)
    C2 = Checkbutton(fm[5], text = "backtrack matrix", variable = CheckVar2, \
                     onvalue = 1, offvalue = 0, height=5, \
                     width = 20)   
    C3 = Checkbutton(fm[5], text = "alignment", variable = CheckVar3, \
                     onvalue = 1, offvalue = 0, height=5, \
                     width = 20)    
    C1.pack(side = LEFT)
    C2.pack(side = LEFT)
    C3.pack(side = LEFT)   
    #fram 7
    fm.append(Frame(height = 20,width = 400))  
    Button(fm[6],text = 'Calculate ',command =lambda : btn_compute(strA,strB,cdel,cins,csub,CheckVar1,CheckVar2,CheckVar3,text)).pack()
    #pack the frame
    fm[0].pack()
    fm[1].pack()
    fm[2].pack()  
    fm[3].pack()
    fm[4].pack()
    fm[5].pack()
    fm[6].pack()    
  
    root.mainloop()
