<template>
  <div class="app-container" :class="{ 'lang-en': isEnglish }">
    <el-form
      :model="queryParams"
      ref="queryForm"
      size="small"
      :inline="true"
      v-show="showSearch"
      :label-width="isEnglish ? '100px' : 'auto'"
      label-position="left"
      :class="{ 'form-en': isEnglish }"
    >
      <el-form-item :label="$t('menuManagement.菜单名称')" prop="menuName">
        <el-input
          v-model="queryParams.menuName"
          :placeholder="$t('menuManagement.请输入菜单名称')"
          clearable
          @keyup.enter.native="handleQuery"
        />
      </el-form-item>
      <el-form-item :label="$t('menuManagement.状态')" prop="status">
        <el-select
          v-model="queryParams.status"
          :placeholder="$t('menuManagement.菜单状态')"
          clearable
        >
          <el-option
            v-for="dict in dict.type.sys_normal_disable"
            :key="dict.value"
            :label="dict.label"
            :value="dict.value"
          />
        </el-select>
      </el-form-item>
      <el-form-item>
        <el-button
          type="primary"
          icon="el-icon-search"
          size="mini"
          @click="handleQuery"
          >{{ $t("menuManagement.搜索") }}</el-button
        >
        <el-button icon="el-icon-refresh" size="mini" @click="resetQuery">{{
          $t("menuManagement.重置")
        }}</el-button>
      </el-form-item>
    </el-form>

    <el-row :gutter="10" class="mb8">
      <el-col :span="1.5">
        <el-button
          type="primary"
          plain
          icon="el-icon-plus"
          size="mini"
          @click="handleAdd"
          v-hasPermi="['system:menu:add']"
          >{{ $t("menuManagement.新增") }}</el-button
        >
      </el-col>
      <el-col :span="1.5">
        <el-button
          type="info"
          plain
          icon="el-icon-sort"
          size="mini"
          @click="toggleExpandAll"
          >{{ $t("menuManagement.展开/折叠") }}</el-button
        >
      </el-col>
      <right-toolbar
        :showSearch.sync="showSearch"
        @queryTable="getList"
      ></right-toolbar>
    </el-row>

    <el-table
      v-if="refreshTable"
      v-loading="loading"
      :data="menuList"
      row-key="menuId"
      :default-expand-all="isExpandAll"
      :tree-props="{ children: 'children', hasChildren: 'hasChildren' }"
    >
      <el-table-column
        prop="menuName"
        :label="$t('menuManagement.菜单名称')"
        :show-overflow-tooltip="true"
        width="160"
      ></el-table-column>
      <el-table-column
        prop="icon"
        :label="$t('menuManagement.图标')"
        align="center"
        width="100"
      >
        <template slot-scope="scope">
          <svg-icon :icon-class="scope.row.icon" />
        </template>
      </el-table-column>
      <el-table-column
        prop="orderNum"
        :label="$t('menuManagement.排序')"
        width="60"
      ></el-table-column>
      <el-table-column
        prop="perms"
        :label="$t('menuManagement.权限标识')"
        :show-overflow-tooltip="true"
      ></el-table-column>
      <el-table-column
        prop="component"
        :label="$t('menuManagement.组件路径')"
        :show-overflow-tooltip="true"
      ></el-table-column>
      <el-table-column
        prop="status"
        :label="$t('menuManagement.状态')"
        width="80"
      >
        <template slot-scope="scope">
          <dict-tag
            :options="dict.type.sys_normal_disable"
            :value="scope.row.status"
          />
        </template>
      </el-table-column>
      <el-table-column
        :label="$t('menuManagement.创建时间')"
        align="center"
        prop="createTime"
      >
        <template slot-scope="scope">
          <span>{{ parseTime(scope.row.createTime) }}</span>
        </template>
      </el-table-column>
      <el-table-column
        :label="$t('menuManagement.操作')"
        align="center"
        class-name="small-padding fixed-width"
      >
        <template slot-scope="scope">
          <el-button
            size="mini"
            type="text"
            icon="el-icon-edit"
            @click="handleUpdate(scope.row)"
            v-hasPermi="['system:menu:edit']"
            >{{ $t("menuManagement.修改") }}</el-button
          >
          <el-button
            size="mini"
            type="text"
            icon="el-icon-plus"
            @click="handleAdd(scope.row)"
            v-hasPermi="['system:menu:add']"
            >{{ $t("menuManagement.新增") }}</el-button
          >
          <el-button
            size="mini"
            type="text"
            icon="el-icon-delete"
            @click="handleDelete(scope.row)"
            v-hasPermi="['system:menu:remove']"
            >{{ $t("menuManagement.删除") }}</el-button
          >
        </template>
      </el-table-column>
    </el-table>

    <!-- 添加或修改菜单对话框 -->
    <el-dialog :title="title" :visible.sync="open" width="850px" append-to-body>
      <el-form
        ref="form"
        :model="form"
        :rules="rules"
        :label-width="isEnglish ? '150px' : '100px'"
        label-position="left"
        :class="{ 'form-en': isEnglish }"
      >
        <el-row>
          <el-col :span="24">
            <el-form-item
              :label="$t('menuManagement.上级菜单')"
              prop="parentId"
            >
              <treeselect
                v-model="form.parentId"
                :options="menuOptions"
                :normalizer="normalizer"
                :show-count="true"
                :placeholder="$t('menuManagement.选择上级菜单')"
              />
            </el-form-item>
          </el-col>
          <el-col :span="24">
            <el-form-item
              :label="$t('menuManagement.菜单类型')"
              prop="menuType"
            >
              <el-radio-group v-model="form.menuType">
                <el-radio label="M">{{ $t("menuManagement.目录") }}</el-radio>
                <el-radio label="C">{{ $t("menuManagement.菜单") }}</el-radio>
                <el-radio label="F">{{ $t("menuManagement.按钮") }}</el-radio>
              </el-radio-group>
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType != 'F'">
            <el-form-item :label="$t('menuManagement.菜单图标')" prop="icon">
              <el-popover
                placement="bottom-start"
                width="460"
                trigger="click"
                @show="$refs['iconSelect'].reset()"
              >
                <IconSelect
                  ref="iconSelect"
                  @selected="selected"
                  :active-icon="form.icon"
                />
                <el-input
                  slot="reference"
                  v-model="form.icon"
                  :placeholder="$t('menuManagement.点击选择图标')"
                  readonly
                >
                  <svg-icon
                    v-if="form.icon"
                    slot="prefix"
                    :icon-class="form.icon"
                    style="width: 25px"
                  />
                  <i
                    v-else
                    slot="prefix"
                    class="el-icon-search el-input__icon"
                  />
                </el-input>
              </el-popover>
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item
              :label="$t('menuManagement.显示排序')"
              prop="orderNum"
            >
              <el-input-number
                v-model="form.orderNum"
                controls-position="right"
                :min="0"
              />
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item
              :label="$t('menuManagement.菜单名称')"
              prop="menuName"
            >
              <el-input
                v-model="form.menuName"
                :placeholder="$t('menuManagement.请输入菜单名称')"
              />
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType == 'C'">
            <el-form-item prop="routeName">
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.默认不填则和路由地址相同：如地址为：`user`，则名称为`User`（注意：因为router会删除名称相同路由，为避免名字的冲突，特殊情况下请自定义，保证唯一性）'
                    )
                  "
                  placement="top"
                >
                  <el-icon><question-filled /></el-icon>
                </el-tooltip>
                {{ $t("menuManagement.路由名称") }}
              </span>
              <el-input
                v-model="form.routeName"
                :placeholder="$t('menuManagement.请输入路由名称')"
              />
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType != 'F'">
            <el-form-item prop="isFrame">
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.选择是外链则路由地址需要以`http(s)://`开头'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.是否外链") }}
              </span>
              <el-radio-group v-model="form.isFrame">
                <el-radio :label="0">{{ $t("menuManagement.是") }}</el-radio>
                <el-radio :label="1">{{ $t("menuManagement.否") }}</el-radio>
              </el-radio-group>
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType != 'F'">
            <el-form-item prop="path">
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.访问的路由地址，如：`user`，如外网地址需内链访问则以`http(s)://`开头'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.路由地址") }}
              </span>
              <el-input
                v-model="form.path"
                :placeholder="$t('menuManagement.请输入路由地址')"
              />
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType == 'C'">
            <el-form-item prop="component">
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.访问的组件路径，如：`system/user/index`，默认在`views`目录下'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.组件路径") }}
              </span>
              <el-input
                v-model="form.component"
                :placeholder="$t('menuManagement.请输入组件路径')"
              />
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType != 'M'">
            <el-form-item prop="perms">
              <el-input
                v-model="form.perms"
                :placeholder="$t('menuManagement.请输入权限标识')"
                maxlength="100"
              />
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.控制器中定义的权限字符，如：@PreAuthorize(`@ss.hasPermi(\'system:user:list\')`)'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.权限字符") }}
              </span>
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType == 'C'">
            <el-form-item prop="query">
              <el-input
                v-model="form.query"
                :placeholder="$t('menuManagement.请输入路由参数')"
                maxlength="255"
              />
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.访问路由的默认传递参数，如：`{&quot;id&quot;: 1, &quot;name&quot;: &quot;ry&quot;}`'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.路由参数") }}
              </span>
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType == 'C'">
            <el-form-item prop="isCache">
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.选择是则会被`keep-alive`缓存，需要匹配组件的`name`和地址保持一致'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.是否缓存") }}
              </span>
              <el-radio-group v-model="form.isCache">
                <el-radio :label="0">{{ $t("menuManagement.缓存") }}</el-radio>
                <el-radio :label="1">{{
                  $t("menuManagement.不缓存")
                }}</el-radio>
              </el-radio-group>
            </el-form-item>
          </el-col>
          <el-col :span="12" v-if="form.menuType != 'F'">
            <el-form-item prop="visible">
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.选择隐藏则路由将不会出现在侧边栏，但仍然可以访问'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.显示状态") }}
              </span>
              <el-radio-group v-model="form.visible">
                <el-radio
                  v-for="dict in dict.type.sys_show_hide"
                  :key="dict.value"
                  :label="dict.value"
                  >{{ dict.label }}</el-radio
                >
              </el-radio-group>
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item prop="status">
              <span slot="label">
                <el-tooltip
                  :content="
                    $t(
                      'menuManagement.选择停用则路由将不会出现在侧边栏，也不能被访问'
                    )
                  "
                  placement="top"
                >
                  <i class="el-icon-question"></i>
                </el-tooltip>
                {{ $t("menuManagement.菜单状态") }}
              </span>
              <el-radio-group v-model="form.status">
                <el-radio
                  v-for="dict in dict.type.sys_normal_disable"
                  :key="dict.value"
                  :label="dict.value"
                  >{{ dict.label }}</el-radio
                >
              </el-radio-group>
            </el-form-item>
          </el-col>
        </el-row>
      </el-form>
      <div slot="footer" class="dialog-footer">
        <el-button type="primary" @click="submitForm">{{
          $t("menuManagement.确 定")
        }}</el-button>
        <el-button @click="cancel">{{ $t("menuManagement.取 消") }}</el-button>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import {
  listMenu,
  getMenu,
  delMenu,
  addMenu,
  updateMenu,
} from "@/api/system/menu";
import Treeselect from "@riophae/vue-treeselect";
import "@riophae/vue-treeselect/dist/vue-treeselect.css";
import IconSelect from "@/components/IconSelect";

export default {
  name: "Menu",
  dicts: ["sys_show_hide", "sys_normal_disable"],
  components: { Treeselect, IconSelect },
  computed: {
    isEnglish() {
      return this.$i18n.locale === "en";
    },
  },
  data() {
    return {
      // 遮罩层
      loading: true,
      // 显示搜索条件
      showSearch: true,
      // 菜单表格树数据
      menuList: [],
      // 菜单树选项
      menuOptions: [],
      // 弹出层标题
      title: "",
      // 是否显示弹出层
      open: false,
      // 是否展开，默认全部折叠
      isExpandAll: false,
      // 重新渲染表格状态
      refreshTable: true,
      // 查询参数
      queryParams: {
        menuName: undefined,
        visible: undefined,
      },
      // 表单参数
      form: {},
      // 表单校验
      rules: {},
    };
  },
  created() {
    this.initRules();
    this.getList();
  },
  methods: {
    /** 初始化表单校验规则 */
    initRules() {
      this.rules = {
        menuName: [
          {
            required: true,
            message: this.$t("menuManagement.菜单名称不能为空"),
            trigger: "blur",
          },
        ],
        orderNum: [
          {
            required: true,
            message: this.$t("menuManagement.菜单顺序不能为空"),
            trigger: "blur",
          },
        ],
        path: [
          {
            required: true,
            message: this.$t("menuManagement.路由地址不能为空"),
            trigger: "blur",
          },
        ],
      };
    },
    // 选择图标
    selected(name) {
      this.form.icon = name;
    },
    /** 查询菜单列表 */
    getList() {
      this.loading = true;
      listMenu(this.queryParams).then((response) => {
        this.menuList = this.handleTree(response.data, "menuId");
        this.loading = false;
      });
    },
    /** 转换菜单数据结构 */
    normalizer(node) {
      if (node.children && !node.children.length) {
        delete node.children;
      }
      return {
        id: node.menuId,
        label: node.menuName,
        children: node.children,
      };
    },
    /** 查询菜单下拉树结构 */
    getTreeselect() {
      listMenu().then((response) => {
        this.menuOptions = [];
        const menu = {
          menuId: 0,
          menuName: this.$t("menuManagement.主类目"),
          children: [],
        };
        menu.children = this.handleTree(response.data, "menuId");
        this.menuOptions.push(menu);
      });
    },
    // 取消按钮
    cancel() {
      this.open = false;
      this.reset();
    },
    // 表单重置
    reset() {
      this.form = {
        menuId: undefined,
        parentId: 0,
        menuName: undefined,
        routeName: undefined,
        icon: undefined,
        menuType: "M",
        orderNum: undefined,
        isFrame: 1,
        isCache: 0,
        visible: "0",
        status: "0",
      };
      this.resetForm("form");
    },
    /** 搜索按钮操作 */
    handleQuery() {
      this.getList();
    },
    /** 重置按钮操作 */
    resetQuery() {
      this.resetForm("queryForm");
      this.handleQuery();
    },
    /** 新增按钮操作 */
    handleAdd(row) {
      this.reset();
      this.getTreeselect();
      if (row != null && row.menuId) {
        this.form.parentId = row.menuId;
      } else {
        this.form.parentId = 0;
      }
      this.open = true;
      this.title = this.$t("menuManagement.添加菜单");
    },
    /** 展开/折叠操作 */
    toggleExpandAll() {
      this.refreshTable = false;
      this.isExpandAll = !this.isExpandAll;
      this.$nextTick(() => {
        this.refreshTable = true;
      });
    },
    /** 修改按钮操作 */
    handleUpdate(row) {
      this.reset();
      this.getTreeselect();
      getMenu(row.menuId).then((response) => {
        this.form = response.data;
        this.open = true;
        this.title = this.$t("menuManagement.修改菜单");
      });
    },
    /** 提交按钮 */
    submitForm: function () {
      this.$refs["form"].validate((valid) => {
        if (valid) {
          if (this.form.menuId != undefined) {
            updateMenu(this.form).then((response) => {
              this.$modal.msgSuccess(this.$t("menuManagement.修改成功"));
              this.open = false;
              this.getList();
            });
          } else {
            addMenu(this.form).then((response) => {
              this.$modal.msgSuccess(this.$t("menuManagement.新增成功"));
              this.open = false;
              this.getList();
            });
          }
        }
      });
    },
    /** 删除按钮操作 */
    handleDelete(row) {
      this.$modal
        .confirm(
          this.$t('menuManagement.是否确认删除名称为"') +
            row.menuName +
            this.$t('menuManagement."的数据项？')
        )
        .then(function () {
          return delMenu(row.menuId);
        })
        .then(() => {
          this.getList();
          this.$modal.msgSuccess(this.$t("menuManagement.删除成功"));
        })
        .catch(() => {});
    },
  },
};
</script>

<style scoped lang="scss">
/* 英文模式下的表单标签样式 */
.lang-en {
  ::v-deep .el-form-item__label {
    font-size: 12px !important;
    padding-right: 12px !important;
  }

  ::v-deep .el-form.form-en {
    .el-form-item__label {
      font-size: 12px !important;
      padding-right: 12px !important;
    }
  }
}
</style>
